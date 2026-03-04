# Implementación del nodo input

El nodo **sp_gen** se encarga de generar una señal que puede ser de tipo: seno, coseno, cuadrada, triangular, de sierra ascendente o sierra descendente. 

Primeramente, se crean dos publicadores, el primero establece el set point para el controlador y el segundo pública el tiempo discreto acumulado 	correspondiente a cada muestra. 
Otros de los componentes internos del programa son:

  • Timer periódico: Dispara el callback de publicación con una frecuencia configurable (publish_frequency).
  
  • Callback de parámetros: Intercepta y valida cambios dinámicos en todos los parámetros declarados.
  
  • Contador de muestras k: Variable entera que lleva el número de muestra actual para calcular el tiempo discreto t = k × Ts.


Por otro lado, el nodo declara seis parámetros en su inicialización. Todos son modificables en tiempo de ejecución. La siguiente tabla describe cada uno:

<img width="785" height="381" alt="Screenshot from 2026-03-04 12-57-10" src="https://github.com/user-attachments/assets/74cd728d-e9c5-4da5-b2d5-7df777e4b724" />


El parámetro signal_type acepta los siguientes valores (cualquier otro es rechazado por el validador):
    
  • sine — Seno: sin(ωt + φ), implementado con math.sin()
  
  • cosine — Coseno: cos(ωt + φ), implementado con math.cos()
  
  • square — Onda cuadrada, implementada con scipy.signal.square()
  
  • triangular — Onda triangular, implementada con scipy.signal.sawtooth(width=0.5)
  
  • sawRight — Sierra ascendente (diente de sierra hacia la derecha), width=1.0
  
  • sawLeft — Sierra descendente (diente de sierra hacia la izquierda), width=0.0

La función privada _load_parameters() lee los seis parámetros con get_parameter(...).value y los almacena en atributos de instancia (self.freq, self.A, self.K, self.P, self.phi, self.signal_type).


