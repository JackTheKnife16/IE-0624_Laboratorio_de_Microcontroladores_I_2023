# Instrucciones para correr el programa

Estas instrucciones serán las necesarias para correr el programa en la placa Arduino NANO 33 BLE y para ver el resultado en la computadora mediante el script serial_com.py.

## Ambiente

Este programa funciona correctamente en el siguiente ambiente:
1. Ubuntu 22.04
2. Arduino IDE 2.1.0: version AppImage

## Instrucciones para cargar el programa

1. En el Arduino IDE ir a: Sketch->Include Library->Add.ZIP Library.
2. Buscar el .zip que se anexa a este laboratorio dentro del src y darle open.
3. Conectar el Arduino NANO 33 BLE a la computadora.
4. En Select Board escoger Arduino NANO 33.
5. En el Arduino IDE ir a: File->Examples->Darkgambler-project-1_inferencing->nano_ble33_sense->laboratorio05.
6. En caso de que esto no aparezca cerrar Arduino IDE y volverlo a abrir.
7. Upload el código.
8. Abrir el monitor serial para ver resultados.

## Instrucciones para comunicar el Arduino NANO 33 BLE con la computadora

1. En el Arduino IDE ver que puerto COM es el que esta conectado al Arduino NANO 33 BLE, suelen ser ACM0, ACM1, ..., supongamos que tenemos la conexión en ACM4.
2. Si por alguna razón no se quiere encontrar el puerto usando Arduino IDE se puede buscar con este comando:
```
ls /dev/tty*
```
2. En caso de tener abierto el Arduino IDE asegurarse de que el monitor serial está cerrado.
3. Abrir una terminal bash en la carpeta src y escribir:
```
python3 serial_com.py -p ACM4
```