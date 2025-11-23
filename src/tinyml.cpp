#include "tinyml.h"

// Globals, for the convenience of one-shot setup.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 16 * 1024; // Adjust size based on your model
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML()
{
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(comfort_model_tflite); // g_model_data is from model_data.h
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

void tiny_ml_task(void *pvParameters)
{
    setupTinyML();

    // === MinMaxScaler parameters (YOUR JSON) ===
    const float FEATURE_SCALE[2] = {
        0.03717472f,  // scale for temperature
        0.0134228189f // scale for humidity
    };
    const float FEATURE_SHIFT[2] = {
        -0.74721187f, // shift for temperature
        -0.24832214f  // shift for humidity
    };
    SensorData recv;
    while (1)
    {
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            recv = data;
            xSemaphoreGive(xSensorMutex);
        }
        float ta = recv.temperature; // °C
        float rh = recv.humidity;    // %

        // ===== 1) CLIP INPUT =====
        if (ta < 20.1f)
            ta = 20.1f;
        if (ta > 47.0f)
            ta = 47.0f;
        if (rh < 18.5f)
            rh = 18.5f;
        if (rh > 93.0f)
            rh = 93.0f;

        // ===== 2) MinMax normalization =====
        float ta_norm = ta * FEATURE_SCALE[0] + FEATURE_SHIFT[0];
        float rh_norm = rh * FEATURE_SCALE[1] + FEATURE_SHIFT[1];

        // ===== 3) WRITE INTO INPUT TENSOR =====
        input->data.f[0] = ta_norm;
        input->data.f[1] = rh_norm;

        // ===== 4) INFER =====
        if (interpreter->Invoke() != kTfLiteOk)
        {
            Serial.println("Invoke failed!");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        // ===== 5) READ OUTPUT TENSOR =====
        float out0 = output->data.f[0]; // Cool
        float out1 = output->data.f[1]; // Neutral
        float out2 = output->data.f[2]; // Warm

        // ===== 6) FIND BEST CLASS =====
        float best = out0;
        int best_class = 0;

        if (out1 > best)
        {
            best = out1;
            best_class = 1;
        }
        if (out2 > best)
        {
            best = out2;
            best_class = 2;
        }

        // ===== 7) MAP CLASS → TEXT =====
        const char *comfort_text;
        switch (best_class)
        {
        case 0:
            comfort_text = "A little bit cool";
            break;
        case 1:
            comfort_text = "Neutral";
            break;
        case 2:
            comfort_text = "Quite hot";
            break;
        }

        // ===== 8) PRINT FULL INFO =====
        Serial.print("[TinyML] Temp=");
        Serial.print(ta);
        Serial.print("°C, Hum=");
        Serial.print(rh);
        Serial.print("% → Comfort: ");
        Serial.print(comfort_text);
        Serial.print(" (class ");
        Serial.print(best_class);
        Serial.print(", prob=");
        Serial.print(best, 4);
        Serial.println(")");

        // Raw output (debug)
        Serial.print("  → raw probs: ");
        Serial.print(out0, 4);
        Serial.print(", ");
        Serial.print(out1, 4);
        Serial.print(", ");
        Serial.print(out2, 4);
        Serial.println();

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
