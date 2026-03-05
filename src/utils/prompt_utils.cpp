//
// Created by yisus on 4/3/26.
//

#include "../../includes/utils/prompt_utils.h"

ollama::request create_request(const std::string& model_name, const std::string& prompt,int temperature, int batch_size, int
                               context_size, int seed)
{
    nlohmann::json options;
    /*¡¡¡ATENCION!!!!!
     * Revisa bien si ollama hace caso a los parametros especificados o los ignora
     * Que ollama tiene fama de ignorar parametros que se le especifican.
     */
    options["options"]["temperature"] = temperature;
    options["options"]["num_ctx"] = context_size;
    options["options"]["num_batch"] = batch_size;
    options["options"]["seed"] = seed;

    ollama::request req(model_name, prompt, options,false);
    req["logprobs"] = true;
    req["verbose"]  = true;
    return req;
}

