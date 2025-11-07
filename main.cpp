/**
 * @file main.cpp
 * @brief Decodificador de Protocolo Industrial PRT-7
 * @author Sistema de Decodificación
 * @date 2025
 * 
 * Este programa implementa un decodificador para el protocolo PRT-7 que lee
 * tramas desde un puerto serial (Arduino) y decodifica mensajes ocultos usando
 * listas doblemente enlazadas y listas circulares.
 */

#include <iostream>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <termios.h>
#endif

// ============================================================================
// DECLARACIONES FORWARD
// ============================================================================

class ListaDeCarga;
class RotorDeMapeo;

// ============================================================================
// CLASE BASE ABSTRACTA - TramaBase
// ============================================================================

/**
 * @class TramaBase
 * @brief Clase base abstracta para todas las tramas del protocolo PRT-7
 * 
 * Define la interfaz común para las tramas LOAD y MAP mediante polimorfismo.
 */
class TramaBase {
public:
    /**
     * @brief Método virtual puro para procesar la trama
     * @param carga Puntero a la lista de carga donde se almacenan los datos
     * @param rotor Puntero al rotor de mapeo que realiza la decodificación
     */
    virtual void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) = 0;
    
    /**
     * @brief Destructor virtual para permitir polimorfismo correcto
     */
    virtual ~TramaBase() {}
};

// ============================================================================
// NODOS PARA ESTRUCTURAS DE DATOS
// ============================================================================

/**
 * @struct NodoRotor
 * @brief Nodo para la lista circular del rotor de mapeo
 */
struct NodoRotor {
    char dato;              ///< Carácter almacenado (A-Z)
    NodoRotor* siguiente;   ///< Puntero al siguiente nodo
    NodoRotor* previo;      ///< Puntero al nodo previo
    
    /**
     * @brief Constructor del nodo
     * @param c Carácter a almacenar
     */
    NodoRotor(char c) : dato(c), siguiente(nullptr), previo(nullptr) {}
};

/**
 * @struct NodoCarga
 * @brief Nodo para la lista doblemente enlazada de carga
 */
struct NodoCarga {
    char dato;              ///< Carácter decodificado
    NodoCarga* siguiente;   ///< Puntero al siguiente nodo
    NodoCarga* previo;      ///< Puntero al nodo previo
    
    /**
     * @brief Constructor del nodo
     * @param c Carácter a almacenar
     */
    NodoCarga(char c) : dato(c), siguiente(nullptr), previo(nullptr) {}
};

// ============================================================================
// ROTOR DE MAPEO - Lista Circular Doblemente Enlazada
// ============================================================================

/**
 * @class RotorDeMapeo
 * @brief Lista circular doblemente enlazada que actúa como disco de cifrado
 * 
 * Implementa una rueda de César que puede rotar para cambiar el mapeo
 * de caracteres dinámicamente.
 */
class RotorDeMapeo {
private:
    NodoRotor* cabeza;  ///< Puntero a la posición 'cero' actual del rotor
    
public:
    /**
     * @brief Constructor que inicializa el rotor con el alfabeto A-Z
     */
    RotorDeMapeo() {
        // Crear lista circular con A-Z
        cabeza = new NodoRotor('A');
        NodoRotor* actual = cabeza;
        
        for (char c = 'B'; c <= 'Z'; c++) {
            NodoRotor* nuevo = new NodoRotor(c);
            actual->siguiente = nuevo;
            nuevo->previo = actual;
            actual = nuevo;
        }
        
        // Cerrar el círculo
        actual->siguiente = cabeza;
        cabeza->previo = actual;
    }
    
    /**
     * @brief Destructor que libera toda la memoria del rotor
     */
    ~RotorDeMapeo() {
        if (!cabeza) return;
        
        NodoRotor* actual = cabeza->siguiente;
        while (actual != cabeza) {
            NodoRotor* temp = actual;
            actual = actual->siguiente;
            delete temp;
        }
        delete cabeza;
    }
    
    /**
     * @brief Rota el rotor N posiciones
     * @param n Número de posiciones a rotar (positivo=adelante, negativo=atrás)
     */
    void rotar(int n) {
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                cabeza = cabeza->siguiente;
            }
        } else if (n < 0) {
            for (int i = 0; i > n; i--) {
                cabeza = cabeza->previo;
            }
        }
    }
    
    /**
     * @brief Obtiene el carácter mapeado según la rotación actual
     * @param in Carácter de entrada
     * @return Carácter mapeado según la posición del rotor
     */
    char getMapeo(char in) {
        // Si no es letra, devolver sin cambios
        if ((in < 'A' || in > 'Z') && (in < 'a' || in > 'z')) {
            return in;
        }
        
        // Convertir a mayúscula
        char c = (in >= 'a' && in <= 'z') ? (in - 32) : in;
        
        // Buscar el carácter en el rotor
        NodoRotor* actual = cabeza;
        int posicionEntrada = 0;
        
        do {
            if (actual->dato == c) {
                break;
            }
            posicionEntrada++;
            actual = actual->siguiente;
        } while (actual != cabeza);
        
        // El mapeo es: avanzar desde la cabeza la misma cantidad de posiciones
        NodoRotor* resultado = cabeza;
        for (int i = 0; i < posicionEntrada; i++) {
            resultado = resultado->siguiente;
        }
        
        return resultado->dato;
    }
};

// ============================================================================
// LISTA DE CARGA - Lista Doblemente Enlazada
// ============================================================================

/**
 * @class ListaDeCarga
 * @brief Lista doblemente enlazada para almacenar caracteres decodificados
 * 
 * Almacena los fragmentos de datos en el orden en que son procesados.
 */
class ListaDeCarga {
private:
    NodoCarga* cabeza;  ///< Primer nodo de la lista
    NodoCarga* cola;    ///< Último nodo de la lista
    
public:
    /**
     * @brief Constructor que inicializa una lista vacía
     */
    ListaDeCarga() : cabeza(nullptr), cola(nullptr) {}
    
    /**
     * @brief Destructor que libera toda la memoria
     */
    ~ListaDeCarga() {
        NodoCarga* actual = cabeza;
        while (actual) {
            NodoCarga* temp = actual;
            actual = actual->siguiente;
            delete temp;
        }
    }
    
    /**
     * @brief Inserta un carácter al final de la lista
     * @param dato Carácter a insertar
     */
    void insertarAlFinal(char dato) {
        NodoCarga* nuevo = new NodoCarga(dato);
        
        if (!cabeza) {
            cabeza = cola = nuevo;
        } else {
            cola->siguiente = nuevo;
            nuevo->previo = cola;
            cola = nuevo;
        }
    }
    
    /**
     * @brief Imprime el mensaje completo ensamblado
     */
    void imprimirMensaje() {
        NodoCarga* actual = cabeza;
        while (actual) {
            std::cout << actual->dato;
            actual = actual->siguiente;
        }
        std::cout << std::endl;
    }
    
    /**
     * @brief Imprime el mensaje con formato detallado
     */
    void imprimirConFormato() {
        NodoCarga* actual = cabeza;
        while (actual) {
            std::cout << "[" << actual->dato << "]";
            actual = actual->siguiente;
        }
    }
};

// ============================================================================
// TRAMAS CONCRETAS
// ============================================================================

/**
 * @class TramaLoad
 * @brief Trama de carga que contiene un fragmento de dato
 * 
 * Representa una trama tipo L,X donde X es un carácter a decodificar.
 */
class TramaLoad : public TramaBase {
private:
    char caracter;  ///< Carácter a procesar
    
public:
    /**
     * @brief Constructor
     * @param c Carácter de la trama
     */
    TramaLoad(char c) : caracter(c) {}
    
    /**
     * @brief Procesa la trama: decodifica el carácter y lo agrega a la lista
     * @param carga Lista donde almacenar el carácter decodificado
     * @param rotor Rotor que realiza el mapeo
     */
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override {
        char decodificado = rotor->getMapeo(caracter);
        carga->insertarAlFinal(decodificado);
        
        std::cout << "Trama recibida: [L," << caracter << "] -> Procesando... -> ";
        std::cout << "Fragmento '" << caracter << "' decodificado como '" 
                  << decodificado << "'. Mensaje: ";
        carga->imprimirConFormato();
        std::cout << std::endl;
    }
};

/**
 * @class TramaMap
 * @brief Trama de mapeo que modifica la rotación del rotor
 * 
 * Representa una trama tipo M,N donde N es el número de rotaciones.
 */
class TramaMap : public TramaBase {
private:
    int rotacion;  ///< Cantidad de rotación a aplicar
    
public:
    /**
     * @brief Constructor
     * @param n Número de posiciones a rotar
     */
    TramaMap(int n) : rotacion(n) {}
    
    /**
     * @brief Procesa la trama: rota el rotor
     * @param carga No se utiliza en esta trama
     * @param rotor Rotor a rotar
     */
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override {
        rotor->rotar(rotacion);
        std::cout << "\nTrama recibida: [M," << rotacion << "] -> Procesando... -> ";
        std::cout << "ROTANDO ROTOR " << (rotacion >= 0 ? "+" : "") << rotacion << ".\n" << std::endl;
    }
};

// ============================================================================
// FUNCIONES DE COMUNICACIÓN SERIAL
// ============================================================================

#ifdef _WIN32
/**
 * @brief Abre el puerto serial en Windows
 * @param puerto Nombre del puerto (ej: "COM3")
 * @return Handle del puerto o INVALID_HANDLE_VALUE si falla
 */
HANDLE abrirPuertoSerial(const char* puerto) {
    HANDLE hSerial = CreateFileA(puerto, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hSerial == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }
    
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    
    SetCommTimeouts(hSerial, &timeouts);
    
    return hSerial;
}

/**
 * @brief Lee una línea del puerto serial en Windows
 * @param hSerial Handle del puerto
 * @param buffer Buffer donde almacenar los datos
 * @param maxLen Tamaño máximo del buffer
 * @return true si se leyó una línea completa
 */
bool leerLineaSerial(HANDLE hSerial, char* buffer, int maxLen) {
    static char bufferInterno[256];
    static int posBuffer = 0;
    
    DWORD bytesLeidos;
    char c;
    
    while (ReadFile(hSerial, &c, 1, &bytesLeidos, NULL) && bytesLeidos > 0) {
        if (c == '\n' || c == '\r') {
            if (posBuffer > 0) {
                bufferInterno[posBuffer] = '\0';
                int i;
                for (i = 0; i < posBuffer && i < maxLen - 1; i++) {
                    buffer[i] = bufferInterno[i];
                }
                buffer[i] = '\0';
                posBuffer = 0;
                return true;
            }
        } else if (posBuffer < 255) {
            bufferInterno[posBuffer++] = c;
        }
    }
    
    return false;
}
#else
/**
 * @brief Abre el puerto serial en Linux
 * @param puerto Nombre del puerto (ej: "/dev/ttyUSB0")
 * @return Descriptor de archivo o -1 si falla
 */
int abrirPuertoSerial(const char* puerto) {
    int fd = open(puerto, O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fd == -1) {
        return -1;
    }
    
    struct termios options;
    tcgetattr(fd, &options);
    
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    
    tcsetattr(fd, TCSANOW, &options);
    
    return fd;
}

/**
 * @brief Lee una línea del puerto serial en Linux
 * @param fd Descriptor del puerto
 * @param buffer Buffer donde almacenar los datos
 * @param maxLen Tamaño máximo del buffer
 * @return true si se leyó una línea completa
 */
bool leerLineaSerial(int fd, char* buffer, int maxLen) {
    static char bufferInterno[256];
    static int posBuffer = 0;
    
    char c;
    int n;
    
    while ((n = read(fd, &c, 1)) > 0) {
        if (c == '\n' || c == '\r') {
            if (posBuffer > 0) {
                bufferInterno[posBuffer] = '\0';
                int i;
                for (i = 0; i < posBuffer && i < maxLen - 1; i++) {
                    buffer[i] = bufferInterno[i];
                }
                buffer[i] = '\0';
                posBuffer = 0;
                return true;
            }
        } else if (posBuffer < 255) {
            bufferInterno[posBuffer++] = c;
        }
    }
    
    return false;
}
#endif

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

/**
 * @brief Función principal del programa
 * @return 0 si finaliza correctamente
 */
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "  DECODIFICADOR PRT-7 - PROTOCOLO INDUSTRIAL" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << std::endl;
    
    // Crear estructuras de datos
    ListaDeCarga* miListaDeCarga = new ListaDeCarga();
    RotorDeMapeo* miRotorDeMapeo = new RotorDeMapeo();
    
    std::cout << "Iniciando Decodificador PRT-7. Conectando a puerto COM..." << std::endl;
    
    // Configurar puerto serial
    const char* nombrePuerto;
    #ifdef _WIN32
        nombrePuerto = "COM3";  // Cambiar según sea necesario
        HANDLE puerto = abrirPuertoSerial(nombrePuerto);
        
        if (puerto == INVALID_HANDLE_VALUE) {
            std::cout << "Error: No se pudo abrir el puerto " << nombrePuerto << std::endl;
            std::cout << "Intente con otro puerto (COM3, COM4, etc.)" << std::endl;
            delete miListaDeCarga;
            delete miRotorDeMapeo;
            return 1;
        }
    #else
        nombrePuerto = "/dev/ttyUSB0";  // Cambiar según sea necesario
        int puerto = abrirPuertoSerial(nombrePuerto);
        
        if (puerto == -1) {
            std::cout << "Error: No se pudo abrir el puerto " << nombrePuerto << std::endl;
            std::cout << "Intente con otro puerto (/dev/ttyUSB0, /dev/ttyACM0, etc.)" << std::endl;
            delete miListaDeCarga;
            delete miRotorDeMapeo;
            return 1;
        }
    #endif
    
    std::cout << "Conexion establecida. Esperando tramas..." << std::endl;
    std::cout << std::endl;
    
    // Buffer para leer líneas
    char buffer[256];
    int tramasRecibidas = 0;
    
    // Bucle principal de procesamiento
    while (true) {
        if (leerLineaSerial(puerto, buffer, 256)) {
            tramasRecibidas++;
            
            // Parsear la trama
            char tipo = buffer[0];
            
            if (tipo == 'L' && buffer[1] == ',') {
                // Trama LOAD
                char caracter = buffer[2];
                if (caracter == 'S' && buffer[3] == 'p' && buffer[4] == 'a' && 
                    buffer[5] == 'c' && buffer[6] == 'e') {
                    caracter = ' ';
                }
                
                TramaBase* trama = new TramaLoad(caracter);
                trama->procesar(miListaDeCarga, miRotorDeMapeo);
                delete trama;
                
            } else if (tipo == 'M' && buffer[1] == ',') {
                // Trama MAP
                int rotacion = 0;
                char* numStr = buffer + 2;
                
                // Convertir manualmente a entero
                bool negativo = false;
                int i = 0;
                if (numStr[0] == '-') {
                    negativo = true;
                    i = 1;
                }
                
                while (numStr[i] >= '0' && numStr[i] <= '9') {
                    rotacion = rotacion * 10 + (numStr[i] - '0');
                    i++;
                }
                
                if (negativo) rotacion = -rotacion;
                
                TramaBase* trama = new TramaMap(rotacion);
                trama->procesar(miListaDeCarga, miRotorDeMapeo);
                delete trama;
                
            } else if (tipo == 'E' && buffer[1] == 'N' && buffer[2] == 'D') {
                // Señal de fin
                break;
            }
        }
        
        // Timeout simple - si no hay más datos, terminar
        #ifdef _WIN32
            Sleep(100);
        #else
            usleep(100000);
        #endif
        
        if (tramasRecibidas > 0 && tramasRecibidas % 15 == 0) {
            // Verificar si hay más datos
            char test;
            #ifdef _WIN32
                DWORD bytesLeidos;
                if (!ReadFile(puerto, &test, 1, &bytesLeidos, NULL) || bytesLeidos == 0) {
                    break;
                }
            #else
                if (read(puerto, &test, 1) <= 0) {
                    break;
                }
            #endif
        }
    }
    
    // Resultado final
    std::cout << "\n---" << std::endl;
    std::cout << "Flujo de datos terminado." << std::endl;
    std::cout << "MENSAJE OCULTO ENSAMBLADO:" << std::endl;
    miListaDeCarga->imprimirMensaje();
    std::cout << "---" << std::endl;
    
    // Limpiar memoria
    std::cout << "Liberando memoria... ";
    delete miListaDeCarga;
    delete miRotorDeMapeo;
    
    #ifdef _WIN32
        CloseHandle(puerto);
    #else
        close(puerto);
    #endif
    
    std::cout << "Sistema apagado." << std::endl;
    
    return 0;
}