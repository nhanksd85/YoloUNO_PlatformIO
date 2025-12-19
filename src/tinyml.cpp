#include "tinyml.h"
#include "LiquidCrystal_I2C.h"

#include <ESP32Servo.h>
Servo myservo;
LiquidCrystal_I2C lcd(33, 16, 2);

// Globals, for the convenience of one-shot setup.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 8 * 1024; // Adjust size based on your model
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML()
{
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite); // g_model_data is from model_data.h
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


void checked()
{
    int pass_count = 0;

    for (int i = 0; i < 3; i++)
    {
        // Servo tap
        myservo.write(30);
        vTaskDelay(pdMS_TO_TICKS(200));
        myservo.write(90);

        vTaskDelay(pdMS_TO_TICKS(300));

        glob_analog_raw = analogRead(1);
        tinyml_ready = true;
        vTaskDelay(pdMS_TO_TICKS(1000));

        Serial.print("[CHECK] ADC=");
        Serial.print(glob_analog_raw);
        Serial.print(" AI=");
        Serial.println(tinyml_result);

        if (tinyml_result > 0.5f)
            pass_count++;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ket qua:");

    lcd.setCursor(0, 1);
    if (pass_count <= 1)
        lcd.print("CHUA CHIN");
    else
        lcd.print("DA CHIN");
}


void tiny_ml_task(void *pvParameters)
{

    setupTinyML();
   
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);

    while (1)
    {


        if (digitalRead(0) == LOW)
        {
            vTaskDelay(100);
            if (digitalRead(0) == LOW)
            {
                checked();
            }
        }


        // // Prepare input data (e.g., sensor readings)
        // // For a simple example, let's assume a single float input
        // input->data.f[0] = glob_temperature;
        // input->data.f[1] = glob_humidity;

        // // Run inference
        // TfLiteStatus invoke_status = interpreter->Invoke();
        // if (invoke_status != kTfLiteOk)
        // {
        //     error_reporter->Report("Invoke failed");
        //     return;
        // }

        // // Get and process output
        // float result = output->data.f[0];
        // Serial.print("Inference result: ");
        // Serial.println(result);

        // vTaskDelay(5000);
        if(tinyml_ready = true){
            float audio_norm =
                ((float)glob_analog_raw - 2048.0f) / 2048.0f;

            input->data.f[0] = audio_norm;

            if (interpreter->Invoke() == kTfLiteOk)
            {
                tinyml_result = output->data.f[0];
            }

            
            tinyml_ready = false;
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
        

        
    
}