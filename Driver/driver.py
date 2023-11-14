import serial
import ctypes

# Establecer la conexión serial, asegúrate de que el puerto coincida con el que estás utilizando
ser = serial.Serial('COM4', 9600)  # Cambia 'COM3' al puerto que estás utilizando

# Obtener el identificador del dispositivo de audio predeterminado
device_id = 0  # Puedes cambiar esto según el dispositivo de audio que quieras controlar

# Obtener el identificador de la interfaz de la API de Windows para el control de volumen
volume_dll = ctypes.windll.winmm
volume_handle = volume_dll.waveOutOpen(None, device_id, None, None, None, 0)

# Función para ajustar el volumen
def set_volume(value):
    # Limitar el valor entre 0 y 100
    volume = max(0, min(value, 100))

    # Convertir el valor de volumen a un rango aceptable por la API de Windows (0-0xFFFF)
    volume_level = int((volume / 100) * 0xFFFF)

    # Llamar a la función de la API de Windows para ajustar el volumen
    volume_dll.waveOutSetVolume(volume_handle, volume_level)

try:
    while True:
        # Leer el valor desde el puerto serial
        volume_value = int(ser.readline().decode().strip())
        
        # Imprimir el valor leído
        print("Volumen:", volume_value)
        
        # Ajustar el volumen en la computadora
        set_volume(volume_value)

except KeyboardInterrupt:
    ser.close()
    print("Programa terminado.")
