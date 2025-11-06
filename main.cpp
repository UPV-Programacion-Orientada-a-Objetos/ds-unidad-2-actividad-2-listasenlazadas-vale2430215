#include "TramaBase.hpp"
#include <iostream>
#include <stdlib.h>

// --- Declaraciones de clases de estructuras de datos ---
class ListaDeCarga {
public:
    void insertarAlFinal(char dato) { 
        std::cout << "DEBUG: Insertando '" << dato << "' en ListaDeCarga." << std::endl;
    }
    void imprimirMensaje() {

     }
};

class RotorDeMapeo {
public:
    void rotar(int N) {

     }
    char getMapeo(char in) {
        return ' ';}
};

// --- Ejemplo de Clase Derivada (TramaLoad Mínima) ---
class TramaLoad : public TramaBase {
private:
    char dato;
public:
    // Constructor (ej. 'L,A')
    TramaLoad(char c) : dato(c) {}

    // Implementación del método virtual puro
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override {
        // Lógica de decodificación de TramaLoad.
        std::cout << "  > [TramaLoad] Procesando: Carga de datos simulada con dato '" << dato << "'." << std::endl;
        carga->insertarAlFinal(dato);
    }
    
    // El destructor de TramaLoad.
    ~TramaLoad() override { 
        std::cout << "  > [TramaLoad] Objeto destruido correctamente." << std::endl; 
    }
};


// ------------------------------------------------------------------
// --- Función Principal ---
// ------------------------------------------------------------------

int main() {
    // Inicialización de las estructuras de datos requeridas.
    ListaDeCarga miListaDeCarga;
    RotorDeMapeo miRotorDeMapeo;

    std::cout << "--- Iniciando simulacion de procesamiento de tramas PRT-7 ---" << std::endl << std::endl;

    // 1. Simulación de lectura serial que produce "L,A"
    
    // **Instanciación Polimórfica:** Creamos un objeto TramaLoad y lo almacenamos en un puntero TramaBase*.
    TramaBase* trama = NULL; 

    // Asignación e Inicialización
    try {
        trama = new TramaLoad('A');

        std::cout << "Trama instanciada como TramaLoad('A')." << std::endl;

        // **Ejecución Polimórfica:** Llama al método procesar() de la clase *derivada*.
        trama->procesar(&miListaDeCarga, &miRotorDeMapeo);

    } catch (std::bad_alloc& e) {
        // Captura básica si falla la asignación de memoria (aunque poco probable aquí).
        std::cerr << "Error de asignacion de memoria: " << e.what() << std::endl;
    }

    // **Limpieza de Memoria Esencial:** El destructor virtual llama a ~TramaLoad() y luego a ~TramaBase().
    if (trama != NULL) {
        std::cout << "Limpiando memoria con 'delete trama;'. Se invoca el destructor virtual." << std::endl;
        delete trama;
        trama = NULL;
    }

    std::cout << std::endl << "--- Fin de la simulacion. ---" << std::endl;
    
    return 0;
}