# RPi5 System Monitor for Large Language Models

[Versión en español](README.md)

Benchmarking tool for measuring LLM inference performance on Raspberry Pi 5. It collects inference metrics (timings, token counts, log-probabilities) and hardware metrics (temperature, frequency, power, memory, throttling) simultaneously during execution.

> **Note:** hardware metrics rely on Raspberry Pi-specific interfaces (`vcgencmd`, `/sys/class/thermal`, etc.) and will not work on other platforms.

---

## Input and output formats

### Input

The program accepts configuration as a JSON file or as an inline argument:

```bash
# From file
./MonitorSystem config_example.json

# Inline
./MonitorSystem --json '{"inference_engine":"OLLAMA", ...}'
```

Configuration JSON fields (see also [`config_example.json`](config_example.json)):

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `inference_engine` | string | yes | `"OLLAMA"` or `"LLAMA"` |
| `test_type` | string | yes | `"TYPE_0"`, `"TYPE_1"` or `"TYPE_2"` |
| `model_path_or_name` | string | yes | Ollama model name or path to a GGUF file |
| `batch_size` | int | yes | Token processing batch size |
| `context_size` | int | yes | Context window size (num_ctx) |
| `seed` | int | yes | Seed for reproducibility |
| `num_prompts` | int | yes | Number of prompts to run |
| `temperature` | float | yes | Sampling temperature |
| `hardware_period` | float | yes | Seconds between hardware samples |
| `anotations` | string | no | Free-text experiment description |
| `ollama_url` | string | no | Ollama server URL (default: `http://localhost:11434`) |

The prompt list comes from the [instruction_following_eval](https://github.com/google-research/google-research/tree/master/instruction_following_eval) dataset by Google Research and is embedded into the binary at compile time, so no external file is needed at runtime.

### Test types

- **TYPE_0**: runs `num_prompts` prompts and collects prompt metrics to file. No hardware metrics.
- **TYPE_1**: runs `num_prompts` prompts sequentially. Collects prompt and hardware metrics in parallel.
- **TYPE_2**: same as TYPE_1 but with a 5-second pause before the first prompt and between each prompt.

### Output

Each run creates a `results/<timestamp_ns>/` directory containing:

```
results/
└── <timestamp_ns>/
    ├── resumen.json                              # run summary
    ├── <id>_prompt_metrics_<model>_test1.jsonl   # per-inference metrics
    └── <id>_hw_metrics_<model>_test1.jsonl       # periodic hardware metrics
```

**`resumen.json`** — run parameters and start/end timestamps.

**`*_prompt_metrics_*.jsonl`** — one JSON line per prompt with:
- `start_timestamp_ns`, `finish_timestamp_ns` — client-side timestamps (ns)
- `total_duration_ns`, `prompt_eval_duration_ns`, `eval_duration_ns`, `load_duration_ns` — inference engine internal timings (ns)
- `prompt_eval_count`, `eval_count` — prompt and response token counts
- `answer` — response text
- `tokenProb` — per-token log-probabilities (Ollama) or probabilities (llama.cpp)
- `engine`, `model`, `prompt_id`

**`*_hw_metrics_*.jsonl`** — one JSON line per sample with:
- `timestamp_` — sample timestamp (ns)
- `temperature_` — CPU temperature (°C)
- `frequency_` — per-core frequency (Hz)
- `voltage_` — core voltage (V)
- `fan_speed_` — fan speed (RPM)
- `internalpower_` — internal power consumption (W) **¹**
- `throttling_` — throttling state (under_voltage, freq_capped, throttled, soft_throttled and their occurrence flags)
- `mem_total_`, `mem_used_`, `mem_percent_` — RAM memory
- `swap_total_`, `swap_used_`, `swap_percent_` — swap
- `cpu_usage_`, `cpu_ticks_` — CPU usage

> **¹** The reported power comes from the Raspberry Pi's internal sensor. It does **not** include HATs or USB peripheral consumption.

---

## Build and run

### Requirements

- CMake ≥ 3.25.1
- C++20
- [vcpkg](https://github.com/microsoft/vcpkg) with the following dependencies:
  - `llama-cpp` ≥ 7146
  - `fmt` ≥ 12.1.0
- For the **OLLAMA** engine: [Ollama](https://ollama.com) installed and running on the system

### Build

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

The resulting binary is **statically linked** (`-static`), so it can be copied and run directly on the Raspberry Pi without installing additional dependencies (except Ollama if using that engine).

### Run

```bash
./MonitorSystem config_example.json
```

The prompt list is embedded in the binary; no external file is needed.

---

## Third-party libraries

| Library | Version/source | Use |
|---------|----------------|-----|
| [ollama-hpp](https://github.com/jmont-dev/ollama-hpp) | header-only in `includes/third_party/` | HTTP client for the Ollama API |
| [nlohmann/json](https://github.com/nlohmann/json) | bundled inside the ollama-hpp header | JSON serialization/deserialization |
| [fmt](https://github.com/fmtlib/fmt) | via vcpkg ≥ 12.1.0 | String formatting |
| [llama.cpp](https://github.com/ggml-org/llama.cpp) | via vcpkg ≥ 7146 | Local inference with GGUF models |
| [instruction_following_eval](https://github.com/google-research/google-research/tree/master/instruction_following_eval) | Google Research | Prompt dataset used in tests |

---

## License

To be determined. This project is part of a Final Degree Project (TFG), which may impose restrictions on the type of license that can be applied.

---

## Notes / Observations

- The codebase mixes English and Spanish (variable names, comments, log messages…). There is no consistent convention.

- Regarding the early git commit history:

  ![git](https://imgs.xkcd.com/comics/git.png)
-   Image Credits: XKCD 1597 by Randall Munroe