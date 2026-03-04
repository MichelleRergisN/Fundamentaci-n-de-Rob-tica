# Fundamentación de Robótica

## Índice

### Descripción del reto
- [Descripción](descripción)

### Implementación nodo input
- [Nodo input](nodoinput.md)

### Implementación nodo control


# Descripción de leer la velocidad desde IDE Arduino con micro ros

- [Codigo en Arduino velocidad y filtro](codigoenarduinovelocidadyfiltro.md)

## Descripción

El reto consiste en controllar la velocidad de un motor DC, su velocidad debe de ser controlada usando una computadora externa, un microcontrolador y un motor driver. Usando Ros2.

Primero, es necesario desarrollar un controlador de velocidad para un motor DC. Para esto al menos dos nodos deben ser implementados: 
/Input, and /Control.
El nodo /Input debe de estar en una computación externa en el nodo /Control 
Se debe de seleccionar donde correr el nodo /Control, si en la computadora o en el micro controlador

<img width="1083" height="341" alt="image" src="https://github.com/user-attachments/assets/767814d2-11f7-4657-b0f1-77463ad059cf" />



### Nodo /Control

-Este nodo debe actuar como transductor, para **enviar información** al controlador del motor y, al mismo tiempo, estimar la velocidad del motor.

-El nodo debe suscribirse y traducir el punto de ajuste en el tema «/set_point» a señales PWM y de dirección al controlador del motor.

-El nodo debe estimar la velocidad del motor utilizando la información de los codificadores y publicar los datos en el tema «/motor_output».

La salida de la MCU al controlador del motor debe ser una «señal PWM» y una señal de dirección.

El ciclo de trabajo PWM y la dirección deben asignarse al  intervalo . 
Donde el signo representa la dirección del motor, es decir, (+) rotación en sentido antihorario y  (-) rotación en sentido horario del motor. 

El  rango del porcentaje del ciclo de trabajo (%)  .
La salida del nodo «/control» (publicar) a la máquina host debe ser la velocidad angular estimada del motor en , en el rango .  Donde el signo (+.-) representa la dirección de rotación obtenida del canal del codificador dual, y  es la velocidad angular, obtenida contando los pulsos del codificador durante un determinado periodo de tiempo.

### Nodo /Input

El nodo «/Input» debe publicar en el tema previamente definido «/set_point».

El nodo debe publicar **diferentes señales de entrada (paso, onda cuadrada, sinusoidal)** al motor.
El usuario debe poder seleccionar y caracterizar estas señales mediante indicadores y parámetros en el archivo de parámetros.
El estudiante puede utilizar archivos de parámetros o ajustar en tiempo real los parámetros de los nodos que se ejecutan en el ordenador.
