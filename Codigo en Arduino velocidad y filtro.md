## Ros2 con Arduino

El código títulado *"VelocidadconFiltro.ino"* implementa un nodo de control de motor DC con encoder usando micro-ROS en un ESP32. 
En resumen hace tres cosas: recibir una señal de velocidad, mover el motor, y publicar la velocidad real medida.


### **Comunicación micro-ROS**
El ESP32 se conecta como nodo ROS2 llamado motor_node. Se suscribe al tópico motor_input (recibe un Float32 de -1.0 a 1.0 que indica qué tan rápido y en qué dirección girar) 
y publica en motor_output (envía la velocidad real medida en RPS).

### **Control del motor**
La función controlarMotor() toma el valor recibido, calcula el PWM proporcional y configura los pines AIN1/AIN2 del driver TB6612 para determinar la dirección de giro. 
El pin STBY se mantiene en HIGH para que el driver esté siempre activo.

**Lectura del encoder**
Dos interrupciones (callback_A y callback_B) detectan los flancos del encoder en cuadratura. Comparando el estado de ambos canales en cada flanco determinan si el motor 
gira hacia adelante (incrementan count) o hacia atrás (decrementan count).

### Cálculo de velocidad a 50Hz
Un timer se dispara cada 20ms. En cada disparo calcula la velocidad real del motor con la fórmula w = count × (1e6 / dt_microsegundos) / TICKS_PER_REV, 
que convierte los ticks del encoder en revoluciones por segundo. Luego resetea count = 0 para el siguiente intervalo.

### Filtro de ventana móvil
Antes de publicar, el valor de RPS pasa por un promedio de las últimas 10 mediciones. Esto elimina el ruido eléctrico del encoder y entrega una señal suave y estable al resto del sistema ROS2.

El filtro utilizado fue un filtro de ventana móvil donde cada muestra de salida es una suma ponderada de las muestras vecinas de la entrada.

Eso es, esencialmente, un promedio. Pero dependiendo de la ventana que uses, el promedio cambia

<img width="598" height="600" alt="image" src="https://github.com/user-attachments/assets/9224bd11-8e7b-4c3e-918d-677af84f1727" />
