#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <iostream>
#include <string>

void SetVolume(float volume) {
    CoInitialize(NULL);

    IMMDeviceEnumerator* deviceEnumerator = NULL;
    IMMDevice* defaultDevice = NULL;
    IAudioEndpointVolume* endpointVolume = NULL;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (LPVOID*)&deviceEnumerator
    );

    if (FAILED(hr)) {
        std::cerr << "Error al obtener el enumerador de dispositivos. Código de error: " << hr << std::endl;
        CoUninitialize();
        return;
    }

    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);

    if (FAILED(hr)) {
        std::cerr << "Error al obtener el dispositivo de audio predeterminado. Código de error: " << hr << std::endl;
        deviceEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = defaultDevice->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL,
        NULL,
        (LPVOID*)&endpointVolume
    );

    if (FAILED(hr)) {
        std::cerr << "Error al activar la interfaz de volumen del dispositivo. Código de error: " << hr << std::endl;
        defaultDevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = endpointVolume->SetMasterVolumeLevelScalar(volume, NULL);

    if (FAILED(hr)) {
        std::cerr << "Error al ajustar el volumen. Código de error: " << hr << std::endl;
    }

    endpointVolume->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
    CoUninitialize();
}


void SendVolumeOverSerial(HANDLE hSerial, float volume) {
    // Convertir el valor de volumen a un entero que se pueda enviar
    int volumeValue = static_cast<int>(volume * 100);

    // Crear una cadena con el valor de volumen
    std::string volumeStr = std::to_string(volumeValue) + '\n';

    // Escribir la cadena en el puerto serie
    DWORD bytesWritten;
    if (WriteFile(hSerial, volumeStr.c_str(), volumeStr.length(), &bytesWritten, NULL)) {
        std::cout << "Volumen enviado exitosamente: " << volume << std::endl;
    }
    else {
        std::cerr << "Error al enviar el volumen por el puerto serie. Código de error: " << GetLastError() << std::endl;
    }
}


int main() {
    // Abrir el puerto serie COM1
    HANDLE hSerial = CreateFile(L"COM4", GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error al abrir el puerto serie. Código de error: " << GetLastError() << std::endl;
        return 1;
    }

    // Configurar la comunicación serie
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error al obtener la configuración del puerto serie. Código de error: " << GetLastError() << std::endl;
        CloseHandle(hSerial);
        return 1;
    }

    // Configurar la velocidad de transmisión (en baudios)
    dcbSerialParams.BaudRate = CBR_9600;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error al configurar la velocidad del puerto serie. Código de error: " << GetLastError() << std::endl;
        CloseHandle(hSerial);
        return 1;
    }


    // Leer e imprimir datos desde el puerto serie
    char buffer[256];
    DWORD bytesRead;

    while (true) {
        if (ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, 0)) {
            if (bytesRead > 0) {
                //// Imprimir los datos leídos
                buffer[bytesRead] = '\0';

                // Buscar el carácter de nueva línea
                char* newlinePos = strchr(buffer, '\n');

                if (newlinePos != nullptr) {
                    // Encontrar la posición del carácter de nueva línea
                    size_t newlineIndex = newlinePos - buffer;

                    // Copiar la parte relevante de la cadena a un nuevo buffer
                    char numberBuffer[32]; // Tamaño suficiente para almacenar un número
                    strncpy_s(numberBuffer, sizeof(numberBuffer), buffer, newlineIndex);

                    // Convertir la cadena a un entero
                    try {
                        int receivedValue = std::stoi(numberBuffer);
                        std::cout << "Número recibido: " << receivedValue << std::endl;
                        float normalizedValue = (receivedValue != 0) ? static_cast<float>(receivedValue) / 1024.0 : 0.0;
                        SetVolume(normalizedValue);
                        SendVolumeOverSerial(hSerial, normalizedValue);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Excepción al convertir a entero: " << e.what() << std::endl;
                    }

                    // Mover el resto de los datos al principio del buffer
                    size_t remainingBytes = bytesRead - (newlineIndex + 1);
                    memmove(buffer, buffer + newlineIndex + 1, remainingBytes);
                    bytesRead = remainingBytes;
                }
                else {
                    std::cout << "Error" << std::endl;
                }
            }
        }
        else {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING) {
                std::cerr << "Error al leer desde el puerto serie. Código de error: " << error << std::endl;
                break;
            }
        }
    }

    // Cerrar el puerto serie
    CloseHandle(hSerial);

    return 0;
}