/*
El código implementa un nodo de control de motor DC con encoder usando micro-ROS en un ESP32. 
En resumen hace tres cosas: recibir una señal de velocidad, mover el motor, y publicar la velocidad real medida.


Comunicación micro-ROS
El ESP32 se conecta como nodo ROS2 llamado motor_node. Se suscribe al tópico motor_input (recibe un Float32 de -1.0 a 1.0 que indica qué tan rápido y en qué dirección girar) 
y publica en motor_output (envía la velocidad real medida en RPS).

Control del motor
La función controlarMotor() toma el valor recibido, calcula el PWM proporcional y configura los pines AIN1/AIN2 del driver TB6612 para determinar la dirección de giro. 
El pin STBY se mantiene en HIGH para que el driver esté siempre activo.

Lectura del encoder
Dos interrupciones (callback_A y callback_B) detectan los flancos del encoder en cuadratura. Comparando el estado de ambos canales en cada flanco determinan si el motor 
gira hacia adelante (incrementan count) o hacia atrás (decrementan count).

Cálculo de velocidad a 50Hz
Un timer se dispara cada 20ms. En cada disparo calcula la velocidad real del motor con la fórmula w = count × (1e6 / dt_microsegundos) / TICKS_PER_REV, 
que convierte los ticks del encoder en revoluciones por segundo. Luego resetea count = 0 para el siguiente intervalo.

Filtro de ventana móvil
Antes de publicar, el valor de RPS pasa por un promedio de las últimas 10 mediciones. Esto elimina el ruido eléctrico del encoder y entrega una señal suave y estable al resto del sistema ROS2.

*/







#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/float32.h>

#define LED_PIN 13
const int pinPWMA = 27;
const int pinAIN1 = 17;
const int pinAIN2 = 18;
const int pinSTBY = 19; 
const int pinENC_A = 12; 
const int pinENC_B = 14; 

const float TICKS_PER_REV = 572.0; 
volatile long encoder_ticks = 0;
long last_encoder_ticks = 0;
unsigned long last_time_us = 0;

// FILTRO DE MEDIA MOVIL
const int WINDOW_SIZE = 7;
float readings[WINDOW_SIZE];
int read_index = 0;
float total_sum = 0;

rcl_subscription_t subscriber;
rcl_publisher_t publisher;
std_msgs__msg__Float32 sub_msg;
std_msgs__msg__Float32 pub_msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void IRAM_ATTR callback_A() {
  if (digitalRead(pinENC_A) != digitalRead(pinENC_B)) encoder_ticks++;
  else encoder_ticks--;
}

void IRAM_ATTR callback_B() {
  if (digitalRead(pinENC_A) == digitalRead(pinENC_B)) encoder_ticks++;
  else encoder_ticks--;
}

void controlarMotor(float v) {
  v = constrain(v, -1.0, 1.0);
  int pwm = (int)(abs(v) * 255);
  if (v > 0) { digitalWrite(pinAIN1, HIGH); digitalWrite(pinAIN2, LOW); }
  else if (v < 0) { digitalWrite(pinAIN1, LOW); digitalWrite(pinAIN2, HIGH); }
  else { digitalWrite(pinAIN1, LOW); digitalWrite(pinAIN2, LOW); }
  ledcWrite(3, pwm);
}

void subscription_callback(const void * msgin) {
  const std_msgs__msg__Float32 * msg_in = (const std_msgs__msg__Float32 *)msgin;
  controlarMotor(msg_in->data);
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  if (timer != NULL) {
    unsigned long current_time_us = micros();
    long current_ticks = encoder_ticks;
    float dt = (current_time_us - last_time_us) / 1000000.0;

    if (dt > 0) {
      long delta_ticks = current_ticks - last_encoder_ticks;
      float raw_rps = ((float)delta_ticks / TICKS_PER_REV) / dt;

      // FILTRO
      total_sum = total_sum - readings[read_index];
      readings[read_index] = raw_rps;
      total_sum = total_sum + readings[read_index];
      read_index = (read_index + 1) % WINDOW_SIZE;
      float filtered_rps = total_sum / WINDOW_SIZE;

      pub_msg.data = filtered_rps;
      RCSOFTCHECK(rcl_publish(&publisher, &pub_msg, NULL));

      last_time_us = current_time_us;
      last_encoder_ticks = current_ticks;
    }
  }
}

void setup() {
  set_microros_transports();

  pinMode(LED_PIN, OUTPUT);
  pinMode(pinAIN1, OUTPUT); pinMode(pinAIN2, OUTPUT); pinMode(pinSTBY, OUTPUT);
  digitalWrite(pinSTBY, HIGH); 

  pinMode(pinENC_A, INPUT_PULLUP);
  pinMode(pinENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinENC_A), callback_A, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinENC_B), callback_B, FALLING);

  ledcSetup(3, 500, 8);
  ledcAttachPin(pinPWMA, 3);
  
  // Inicializar arreglo de filtro
  for (int i = 0; i < WINDOW_SIZE; i++) readings[i] = 0;

  last_time_us = micros();
  
  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  RCCHECK(rclc_node_init_default(&node, "motor_node", "", &support));

  RCCHECK(rclc_subscription_init_default(&subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "motor_input"));
  RCCHECK(rclc_publisher_init_default(&publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "motor_output"));

  RCCHECK(rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(20), &timer_callback));

  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &sub_msg, &subscription_callback, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
}

void loop() {
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));
}
