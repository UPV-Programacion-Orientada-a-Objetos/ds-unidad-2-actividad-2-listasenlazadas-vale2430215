/**
 * @file TramaBase.hpp
 * @brief Define la clase base abstracta para todas las tramas del protocolo PRT-7.
 */

#ifndef TRAMA_BASE_HPP
#define TRAMA_BASE_HPP

// Declaraciones adelantadas para evitar inclusiones mutuas y errores de compilación.
// Las clases ListaDeCarga y RotorDeMapeo se definirán más tarde.
class ListaDeCarga;
class RotorDeMapeo;

/**
 * @class TramaBase
 * @brief Clase base abstracta que define la interfaz polimórfica para procesar tramas.
 *
 * Contiene el método virtual puro 'procesar' que cada trama concreta (Load o Map)
 * debe implementar con su lógica específica.
 */
class TramaBase {
public:
    /**
     * @brief Método virtual puro para procesar la trama.
     * @param carga Puntero a la ListaDeCarga (donde se ensambla el mensaje).
     * @param rotor Puntero al RotorDeMapeo (el disco de cifrado circular).
     *
     * Este es el núcleo del polimorfismo. La implementación específica (carga de datos
     * o rotación del rotor) se define en las clases derivadas.
     */
    virtual void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) = 0;

    /**
     * @brief Destructor virtual.
     *
     * Asegura que el destructor correcto de la clase derivada sea llamado cuando
     * se hace 'delete' a través de un puntero a la clase base ('TramaBase*'),
     * previniendo fugas de memoria.
     */
    virtual ~TramaBase() {
        // Un destructor virtual puro es a veces necesario, pero para C++ estándar
        // y la implementación de la jerarquía, un destructor virtual normal es suficiente
        // y necesario para la correcta limpieza.
    }
};

#endif // TRAMA_BASE_HPP