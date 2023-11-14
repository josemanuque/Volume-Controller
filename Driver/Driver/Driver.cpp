/***
 * This example expects the serial port has a loopback on it.
 *
 * Alternatively, you could use an Arduino:
 *
 * <pre>
 *  void setup() {
 *    Serial.begin(<insert your baudrate here>);
 *  }
 *
 *  void loop() {
 *    if (Serial.available()) {
 *      Serial.write(Serial.read());
 *    }
 *  }
 * </pre>
 */

#include <string>
#include <iostream>
#include <cstdio>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

 // OS Specific sleep
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial.h"

using std::string;
using std::exception;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::vector;

void my_sleep(unsigned long milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds); // 100 ms
#else
    usleep(milliseconds * 1000); // 100 ms
#endif
}

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




void enumerate_ports()
{
	vector<serial::PortInfo> devices_found = serial::list_ports();

	vector<serial::PortInfo>::iterator iter = devices_found.begin();

	while (iter != devices_found.end())
	{
		serial::PortInfo device = *iter++;

		printf("(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(),
			device.hardware_id.c_str());
	}
}


int main() {
    serial::Serial mySerial;
    string port = "";
    cout << "Enter the COM Port (-1 to search ports): ";
    cin >> port;

    if (port == "-1") {
        enumerate_ports();
        cout << "Enter the COM Port: ";
        cin >> port;
    }
    try {
        serial::Serial mySerial(port, 9600);
        if (mySerial.isOpen()) {
            cout << "Port opened successfully" << endl;
            cout << "Listening..." << endl;
        }
        else {
            cout << "Error opening port" << endl;
            return -1;
        }
        while (true) {
            try {
                string line = mySerial.readline();
                int receivedValue = std::stoi(line);
                cout << "Read number: " << receivedValue << endl;
                float normalizedValue = (receivedValue != 0) ? static_cast<float>(receivedValue) / 1024.0 : 0.0;
                SetVolume(normalizedValue);
                int volumeValue = static_cast<int>(normalizedValue * 100);
                string volumeStr = std::to_string(volumeValue);
                cout << "Sending volume percentage: " << volumeStr << " %" << endl;
                cout << "____________________________" << endl;
                mySerial.write(volumeStr + "\n");
            }
            catch (const std::exception& e) {
                std::cerr << "Cannot parse string to int: " << e.what() << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        // Captura cualquier excepción derivada de std::exception
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
	
	return 0;
}