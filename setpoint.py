#!/usr/bin/env python3
import rclpy, time
from rclpy.node import Node
from std_msgs.msg import Float32
from rcl_interfaces.msg import SetParametersResult
from numpy import sin, cos, pi
from scipy.signal import square, sawtooth

class MotorSetPoint(Node):
    def __init__(self):
        super().__init__("signal_generator")
        self.get_logger().info("Generador de Referencia (Setpoint) iniciado")

        # Parametros iniciales
        self.declare_parameter("amplitude", 4.0)  # RPS deseados
        self.declare_parameter("frequency", 0.5)  # Hz
        self.declare_parameter("offset", 0.0)
        self.declare_parameter("sampling_time", 0.11) # 50Hz para match con Arduino
        self.declare_parameter("type", 3)  # Por defecto onda cuadrada para pruebas PID

        self.update_params()

        # Un solo publicador hacia el 'Subscriber' del diagrama
        self.pub_setpoint = self.create_publisher(Float32, 'motor_input', 10)
        
        # Feedback del motor (Subscriber en el diagrama)
        #self.subscription = self.create_subscription(Float32, 'motor_output', self.listener_callback, 10)

        self.create_timer(self.sampling_time, self.generator_callback)
        self.add_on_set_parameters_callback(self.parameters_callback)

        self.t0 = time.time()

    def update_params(self):
        self.A = self.get_parameter("amplitude").value
        self.f = self.get_parameter("frequency").value
        self.K = self.get_parameter("offset").value
        self.sampling_time = self.get_parameter("sampling_time").value
        self.signal_type = self.get_parameter("type").value

    def generator_callback(self):
        t = time.time() - self.t0
        msg = Float32()
        
        # u(t) = K + A * signal(2 * pi * f * t)
        w_t = 2.0 * pi * self.f * t

        if self.signal_type == 0: # Escalón
            msg.data = float(self.A if t > 2.0 else 0.0) + self.K
        elif self.signal_type == 1: # Seno
            msg.data = float(self.A * sin(w_t) + self.K)
        elif self.signal_type == 3: # Cuadrada
            msg.data = float(self.A * square(w_t) + self.K)
        elif self.signal_type == 4: # Triangular
            msg.data = float(self.A * sawtooth(w_t, 0.5) + self.K)
        
        self.pub_setpoint.publish(msg)

    def parameters_callback(self, params):
        for param in params:
            if param.name == 'amplitude': self.A = param.value
            elif param.name == 'frequency': self.f = param.value
            elif param.name == 'type': self.signal_type = param.value
        return SetParametersResult(successful=True)

    def listener_callback(self, msg):
        # Opcional: imprimir en terminal para ver el error en tiempo real
        pass

def main(args=None):
    rclpy.init(args=args)
    node = MotorSetPoint()
    try: rclpy.spin(node)
    except KeyboardInterrupt: pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == "__main__":
    main()
