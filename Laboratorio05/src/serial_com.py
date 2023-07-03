import serial
import csv
import argparse

# Crea un objeto ArgumentParser
parser = argparse.ArgumentParser(description='Este script abre la comunicacion serial con el'
                                 ' microcontrolador para recibir los datos')

# Agrega un argumento de tipo string especificamente el nombre del movimiento que se va a realizar
parser.add_argument('-b', '--baudios',
                    type=int, default=115200,
                    help="baudios para la comunicacion serial, por defecto es 115200")

# Agrega un argumento de tipo string especificamente el puerto serial, se puede obtener en el
# arduino IDE o usando el comando ls /dev/tty*, pero es mas sencillo de ver en el arduino IDE
parser.add_argument('-p', '--puerto',
                    type=str, help='puerto abierto para la comunicacion serial, lo que viene despues'
                    ' del tty, por ejemplo: ACM2, esto se puede ver en el arduino IDE',
                    required=True)

# Parsea los argumentos de la línea de comandos
args = parser.parse_args()

# el baud rate, este es un valor que debe ser el mismo que aparece en el
# archivo arduino.
baud_rate = args.baudios
# puerto serial que esta abierto
port_name = args.puerto

# este valor de puerto se reemplaza por el que muestra arduino ide o bien se puede
# encontrar usando el comando bash: ls /dev/tty*
port = '/dev/tty' + port_name

# muestra los mensajes que vienen del arduino
while True:
    # Abre el puerto serie
    ser = serial.Serial(port, baud_rate)
    print(ser.readline().decode())

