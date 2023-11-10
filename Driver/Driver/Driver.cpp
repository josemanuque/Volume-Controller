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

int main() {
    // Abrir el puerto serie COM1
    HANDLE hSerial = CreateFile(L"COM4", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

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
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, 0)) {
            if (bytesRead > 0) {
                // Imprimir los datos leídos
                //std::cout.write(buffer, bytesRead);
                buffer[bytesRead] = '\0'; // Asegúrate de que el buffer esté terminado con un carácter nulo
                char* newlinePos = std::remove(buffer, buffer + bytesRead, '\n');
                *newlinePos = '\0'; // Terminar la cadena después de eliminar el carácter de nueva línea

                std::cout << "Datos recibidos: " << buffer << std::endl;
                
                try {
                    int receivedValue = std::stoi(std::string(buffer, bytesRead));
                    float normalizedValue = (receivedValue != 0) ? static_cast<float>(receivedValue) / 1024.0 : 0.0;
                    std::cout << "normalizedValue: " << normalizedValue << std::endl;
                    SetVolume(normalizedValue);
                }
                catch (const std::exception& e) {
                    //std::cerr << "Excepción al procesar los datos: " << e.what() << std::endl;
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