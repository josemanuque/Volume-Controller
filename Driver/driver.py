import serial
import ctypes

# Establecer la conexión serial, asegúrate de que el puerto coincida con el que estás utilizando
ser = serial.Serial('COM4', 9600)  # Cambia 'COM3' al puerto que estás utilizando

# Función para ajustar el volumen
def set_volume(value):
    # Limitar el valor entre 0 y 100
    volume = max(0, min(value, 100))
    
    # Usar la biblioteca ctypes para llamar a la función de la API de Windows para ajustar el volumen
    ctypes.windll.volume.SetVolume(int(volume * 655.35))

try:
    while True:
        # Leer el valor desde el puerto serial
        volume_value = int(ser.readline().decode().strip())
        
        # Imprimir el valor leído
        print("Volumen:", volume_value)
        
        # Ajustar el volumen en la computadora
        #set_volume(volume_value)

except KeyboardInterrupt:
    ser.close()
    print("Programa terminado.")
