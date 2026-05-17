# RPi5 Monitor del sistema para modelos largos de lenguaje

[English version](README_eng.md)

Herramienta de benchmarking para medir el rendimiento de modelos de lenguaje (LLM) en Raspberry Pi 5. Recoge métricas de inferencia (tiempos, tokens, log-probabilidades) y métricas hardware (temperatura, frecuencia, potencia, memoria, throttling) de forma simultánea durante la ejecución.

> **Nota:** las métricas hardware utilizan interfaces específicas de Raspberry Pi (`vcgencmd`, `/sys/class/thermal`, etc.) y no funcionarán en otras plataformas.

---

## Formatos de entrada y salida

### Entrada

El programa acepta la configuración como fichero JSON o como argumento inline:

```1
# Desde fichero
./MonitorSystem config_example.json

# Inline
./MonitorSystem --json '{"inference_engine":"OLLAMA", ...}'
```

Campos del JSON de configuración (ver también [`config_example.json`](config_example.json)):

| Campo | Tipo | Requerido | Descripción |
|-------|------|-----------|-------------|
| `inference_engine` | string | sí | `"OLLAMA"`, `"LLAMA"` o `"HAILO_OLLAMA"` |
| `test_type` | string | sí | `"TYPE_0"`, `"TYPE_1"` o `"TYPE_2"` |
| `model_path_or_name` | string | sí | Nombre del modelo Ollama/Hailo o ruta al fichero GGUF |
| `batch_size` | int | sí | Tamaño de batch de procesado de tokens ¹ |
| `context_size` | int | sí | Tamaño de la ventana de contexto (num_ctx) ¹ |
| `seed` | int | sí | Semilla para reproducibilidad (acepta negativos; -1 = aleatoria) |
| `num_prompts` | int | sí | Número de prompts a ejecutar (1–541) |
| `temperature` | float | sí | Temperatura de muestreo |
| `hardware_period` | float | sí | Segundos entre muestras de hardware |
| `annotations` | string/objeto JSON | no | Descripción libre del experimento; si es un objeto JSON se fusiona con los metadatos del modelo |
| `ollama_url` | string | no | URL del servidor Ollama (default: `http://localhost:11434`). Solo para `OLLAMA`. |
| `hailo_server_host` | string | no | Host del servidor hailo-ollama (default: `localhost`). Solo para `HAILO_OLLAMA`. |
| `hailo_server_port` | int | no | Puerto del servidor hailo-ollama (default: `8000`). Solo para `HAILO_OLLAMA`. |

> **¹** `batch_size` y `context_size` se registran en el resumen pero **no tienen efecto en tiempo de ejecución para `HAILO_OLLAMA`**: los modelos Hailo se compilan como ficheros HEF con estos parámetros fijados en tiempo de compilación.

La lista de prompts proviene del dataset [instruction_following_eval](https://github.com/google-research/google-research/tree/master/instruction_following_eval) de Google Research y se embebe en el binario en tiempo de compilación, por lo que no es necesario ningún fichero externo en tiempo de ejecución.

### Tipos de test

- **TYPE_0**: ejecuta `num_prompts` prompts y recoge métricas de prompt en fichero. Sin métricas de hardware.
- **TYPE_1**: ejecuta `num_prompts` prompts de forma secuencial. Recoge métricas de prompt y hardware en paralelo.
- **TYPE_2**: igual que TYPE_1 pero con una pausa de 5 segundos antes del primer prompt y entre cada prompt.

### Salida

Cada ejecución crea una carpeta `results/<timestamp_ns>/` con:

```
results/
└── <timestamp_ns>/
    ├── resumen.json                              # resumen de la ejecución
    ├── <id>_prompt_metrics_<model>_test1.jsonl   # métricas por inferencia
    └── <id>_hw_metrics_<model>_test1.jsonl       # métricas hardware periódicas
```

**`resumen.json`** — parámetros de la ejecución y timestamps de inicio y fin. Incluye `hardware_period` como campo de primer nivel y `annotations` como objeto JSON con los metadatos de arquitectura del modelo bajo la clave `model_info` (`n_params`, `architecture`, `embedding_length`, `n_layers`, `max_context`, `quantization`).

**`*_prompt_metrics_*.jsonl`** — una línea JSON por prompt con:
- `start_timestamp_ns`, `finish_timestamp_ns` — timestamps del cliente (ns)
- `total_duration_ns`, `prompt_eval_duration_ns`, `eval_duration_ns`, `load_duration_ns` — tiempos internos del motor de inferencia (ns)
- `prompt_eval_count`, `eval_count` — tokens de prompt y de respuesta
- `answer` — texto de la respuesta
- `tokenProb` — log-probabilidades por token (Ollama) o probabilidades (llama.cpp)
- `engine`, `model`, `prompt_id`

**Campos a `0` intencionalmente para `HAILO_OLLAMA`:** la API de hailo-ollama solo devuelve `total_duration` y `eval_count` en la respuesta final. Los campos siguientes se fijan a `0` por diseño al no estar disponibles en el servidor:

| Campo | Motivo |
|-------|--------|
| `prompt_eval_count` | No reportado por hailo-ollama |
| `prompt_eval_duration_ns` | No reportado por hailo-ollama |
| `eval_duration_ns` | No reportado por hailo-ollama |
| `load_duration_ns` | No reportado por hailo-ollama |

`tokenProb` se establece a `"NONE"` ya que hailo-ollama no expone log-probabilidades por token.

**`*_hw_metrics_*.jsonl`** — una línea JSON por muestra con:
- `timestamp_` — timestamp de la muestra (ns)
- `temperature_` — temperatura de la CPU (°C)
- `frequency_` — frecuencia por núcleo (Hz)
- `voltage_` — tensión del núcleo (V)
- `fan_speed_` — velocidad del ventilador (RPM)
- `internalpower_` — consumo de potencia interno (W) **¹**
- `throttling_` — estado de throttling (under_voltage, freq_capped, throttled, soft_throttled y sus flags de ocurrencia)
- `mem_total_`, `mem_used_`, `mem_percent_` — memoria RAM
- `swap_total_`, `swap_used_`, `swap_percent_` — swap
- `cpu_usage_`, `cpu_ticks_` — uso de CPU

> **¹** La potencia reportada es la del sensor interno de la Raspberry Pi. **No incluye** el consumo de HATs ni de periféricos USB.

---

## Compilación y ejecución

### Requisitos

- CMake ≥ 3.25.1
- C++20
- [vcpkg](https://github.com/microsoft/vcpkg) con las siguientes dependencias:
  - `llama-cpp` ≥ 7146
  - `fmt` ≥ 12.1.0
- Para el motor **OLLAMA**: [Ollama](https://ollama.com) instalado y ejecutándose en el sistema
- Para el motor **HAILO_OLLAMA**: servidor [hailo-ollama](https://github.com/hailo-ai/hailo_model_zoo_genai) accesible en red (por defecto `localhost:8000`)

### Compilar

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<ruta_a_vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

El binario resultante está enlazado de forma **estática** (`-static`), por lo que puede copiarse y ejecutarse directamente en la Raspberry Pi sin instalar dependencias adicionales (excepto Ollama si se usa ese motor).

### Ejecutar

```bash
./MonitorSystem config_example.json
```

La lista de prompts está embebida en el binario; no se necesita ningún fichero externo.

---

## Librerías de terceros

| Librería | Versión/fuente | Uso |
|----------|----------------|-----|
| [ollama-hpp](https://github.com/jmont-dev/ollama-hpp) | header-only en `includes/third_party/` | Cliente HTTP para la API de Ollama |
| [nlohmann/json](https://github.com/nlohmann/json) | incluida en el header de ollama-hpp | Serialización/deserialización JSON |
| [fmt](https://github.com/fmtlib/fmt) | vía vcpkg ≥ 12.1.0 | Formateo de cadenas |
| [llama.cpp](https://github.com/ggml-org/llama.cpp) | vía vcpkg ≥ 7146 | Inferencia local con modelos GGUF |
| `hailo_http_client.h` | header-only en `includes/third_party/` | Cliente HTTP POSIX ligero para hailo-ollama (sin TLS, sin dependencias externas) |
| [instruction_following_eval](https://github.com/google-research/google-research/tree/master/instruction_following_eval) | Google Research | Dataset de prompts usado en los tests |

---

## Licencia

Pendiente de definir. Este proyecto forma parte de un Trabajo de Fin de Grado (TFG), lo que puede imponer restricciones sobre el tipo de licencia aplicable.

---

## Anotaciones / Observaciones

- El código mezcla inglés y español (nombres de variables, comentarios, mensajes de log…). No hay una convención uniforme.

- Respecto a el historial de commits de git inicial: 

  ![git](https://imgs.xkcd.com/comics/git.png)
- Creditos de la imagen: XKCD 1597 by Randall Munroe