# Cómo Obtener Datos de Rendimiento con ollama.hpp

## Resumen

La librería `ollama.hpp` proporciona acceso completo a todos los datos de rendimiento devueltos por la API de Ollama a través del método `response.as_json()`.

## Datos Disponibles de Rendimiento

La API de Ollama devuelve los siguientes datos de rendimiento en la respuesta JSON:

### 1. **total_duration** (duración total en nanosegundos)
   - Tiempo total desde que se inicia la solicitud hasta que se completa
   - Se convierte a milisegundos dividiendo por 1,000,000
   - Se convierte a segundos dividiendo por 1,000,000,000

```cpp
if (response.as_json().contains("total_duration")) {
    auto total_ns = response.as_json()["total_duration"].get<long long>();
    double total_ms = total_ns / 1000000.0;
    std::cout << "Duración total: " << total_ms << " ms" << std::endl;
}
```

### 2. **load_duration** (tiempo para cargar el modelo)
   - Tiempo que tarda en cargar el modelo LLM
   - También en nanosegundos

```cpp
if (response.as_json().contains("load_duration")) {
    auto load_ns = response.as_json()["load_duration"].get<long long>();
    double load_ms = load_ns / 1000000.0;
    std::cout << "Tiempo de carga: " << load_ms << " ms" << std::endl;
}
```

### 3. **prompt_eval_duration** (tiempo para procesar el prompt)
   - Tiempo que tarda en procesar/evaluar el texto de entrada
   - En nanosegundos

```cpp
if (response.as_json().contains("prompt_eval_duration")) {
    auto prompt_eval_ns = response.as_json()["prompt_eval_duration"].get<long long>();
    double prompt_eval_ms = prompt_eval_ns / 1000000.0;
    std::cout << "Tiempo de evaluación del prompt: " << prompt_eval_ms << " ms" << std::endl;
}
```

### 4. **prompt_eval_count** (número de tokens en el prompt)
   - Cantidad de tokens en el texto de entrada
   - Valor entero

```cpp
if (response.as_json().contains("prompt_eval_count")) {
    auto prompt_tokens = response.as_json()["prompt_eval_count"].get<int>();
    std::cout << "Tokens en prompt: " << prompt_tokens << std::endl;
}
```

### 5. **eval_duration** (tiempo para generar tokens)
   - Tiempo que tarda en generar la respuesta
   - En nanosegundos

```cpp
if (response.as_json().contains("eval_duration")) {
    auto eval_ns = response.as_json()["eval_duration"].get<long long>();
    double eval_ms = eval_ns / 1000000.0;
    std::cout << "Tiempo de generación: " << eval_ms << " ms" << std::endl;
}
```

### 6. **eval_count** (número de tokens generados)
   - Cantidad de tokens en la respuesta generada
   - Valor entero

```cpp
if (response.as_json().contains("eval_count")) {
    auto generated_tokens = response.as_json()["eval_count"].get<int>();
    std::cout << "Tokens generados: " << generated_tokens << std::endl;
}
```

## Cálculos Útiles

### Velocidad de Generación (tokens/segundo)

```cpp
auto eval_ns = response.as_json()["eval_duration"].get<long long>();
auto eval_count = response.as_json()["eval_count"].get<int>();

if (eval_ns > 0) {
    double tokens_per_second = (eval_count / (eval_ns / 1000000000.0));
    std::cout << "Velocidad: " << tokens_per_second << " tokens/segundo" << std::endl;
}
```

### Velocidad de Procesamiento del Prompt (tokens/segundo)

```cpp
auto prompt_eval_ns = response.as_json()["prompt_eval_duration"].get<long long>();
auto prompt_tokens = response.as_json()["prompt_eval_count"].get<int>();

if (prompt_eval_ns > 0) {
    double prompt_tokens_per_sec = (prompt_tokens / (prompt_eval_ns / 1000000000.0));
    std::cout << "Velocidad de prompt: " << prompt_tokens_per_sec << " tokens/segundo" << std::endl;
}
```

## Ejemplo Completo

```cpp
#include "utils/ollama.hpp"
#include <iostream>
#include <iomanip>

void print_performance_metrics(const nlohmann::json& response_json)
{
    std::cout << "\n=== MÉTRICAS DE RENDIMIENTO ===" << std::endl;
    
    if (response_json.contains("total_duration")) {
        auto total_ns = response_json["total_duration"].get<long long>();
        std::cout << "Duración total: " << std::fixed << std::setprecision(2) 
                  << (total_ns / 1000000.0) << " ms" << std::endl;
    }
    
    if (response_json.contains("prompt_eval_count")) {
        std::cout << "Tokens en prompt: " 
                  << response_json["prompt_eval_count"].get<int>() << std::endl;
    }
    
    if (response_json.contains("eval_count")) {
        std::cout << "Tokens generados: " 
                  << response_json["eval_count"].get<int>() << std::endl;
    }
    
    if (response_json.contains("eval_duration") && response_json.contains("eval_count")) {
        auto eval_ns = response_json["eval_duration"].get<long long>();
        auto eval_count = response_json["eval_count"].get<int>();
        if (eval_ns > 0) {
            double tokens_per_second = (eval_count / (eval_ns / 1000000000.0));
            std::cout << "Velocidad: " << std::fixed << std::setprecision(2) 
                      << tokens_per_second << " tokens/segundo" << std::endl;
        }
    }
    
    std::cout << "==============================\n" << std::endl;
}

int main()
{
    ollama::allow_exceptions(true);
    
    // Usar chat o generate
    auto response = ollama::chat("modelo-name", "Tu pregunta aquí");
    
    // Mostrar la respuesta
    std::cout << response << std::endl;
    
    // Mostrar métricas de rendimiento
    print_performance_metrics(response.as_json());
    
    return 0;
}
```

## Notas Importantes

1. **Los datos solo están disponibles cuando stream=false**: Los datos de rendimiento completos solo se devuelven al final de la respuesta cuando la generación no es streaming.

2. **El JSON completo está disponible**: Puedes acceder a cualquier campo JSON devuelto por la API usando `response.as_json()["campo_nombre"]`

3. **Conversiones de tiempo**: Todos los tiempos están en nanosegundos:
   - Milisegundos: dividir por 1,000,000
   - Segundos: dividir por 1,000,000,000

4. **Validación**: Siempre usa `.contains()` para verificar si el campo existe antes de acceder a él

5. **Tipos de datos**: 
   - Duraciones: `long long`
   - Conteos: `int`

## Campos Adicionales

Dependiendo del tipo de solicitud (chat, generate, embedding), puede haber otros campos disponibles:

- `model`: nombre del modelo usado
- `created_at`: marca de tiempo de creación
- `message`: para chat, contiene la respuesta estructurada
- `response`: para generate, contiene la respuesta como texto
- `context`: contexto de la conversación (para maintain state)

Todos estos campos son accesibles a través de `response.as_json()`.

