import serial
import csv

# Establece la conexión serial con el puerto y la velocidad de transmisión de datos
ser = serial.Serial('/tmp/ttyS1', 9600)

# Crea el archivo CSV y escribe la primera fila de encabezado
with open('datos.csv', mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['TIPO (AC/DC)', 'FUENTE', 'TENSION (V)', 'TIEMPO (s)'])

while True:
    # Espera hasta que se reciba un dato por el puerto serial
    if ser.in_waiting > 0:
        # Lee el dato recibido y conviértelo a una cadena
        dato = ser.readline().decode('utf-8').strip()
        
        # Separa los datos por comas y los convierte a una lista
        datos_lista = dato.split(',')
        value =  int(datos_lista[3]) / 1000000
        datos_lista[3] = str(value)
        
        # Escribe los datos en el archivo CSV
        with open('datos.csv', mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(datos_lista)
            
        # Imprime el dato recibido en la consola
        print(datos_lista)
