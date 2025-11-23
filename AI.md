# Thermal Comfort Model – How It Works

## 1. Model Input – Nhận dữ liệu gì?

Mô hình nhận 2 giá trị đầu vào (đã chuẩn hóa):

- input[0]: Temperature (normalized)
- input[1]: Humidity (normalized)

Chuẩn hóa dùng MinMaxScaler theo công thức:

    x_norm = x * scale + shift

(scale và shift nằm trong norm_stats_minmax.json)

## 2. Quy trình xử lý trước khi đưa vào model

1) Đọc sensor:

    float ta = glob_temperature;
    float rh = glob_humidity;

2) Giới hạn để tránh sensor lỗi:

- Temp: [20.1 → 47]
- Humidity: [18.5 → 93]

3) Normalize:

    ta_norm = ta * scale[0] + shift[0]
    rh_norm = rh * scale[1] + shift[1]

4) Ghi vào input tensor:

    input->data.f[0] = ta_norm;
    input->data.f[1] = rh_norm;

## 3. Model Output – Trả ra gì?

Model trả về 3 xác suất:

- out[0] = Cool
- out[1] = Neutral
- out[2] = Warm

Ví dụ:

    [0.12, 0.73, 0.15]

## 4. Chọn kết quả tốt nhất

    best_class = argmax(out)
    best_prob  = max(out)

## 5. Map class → Comfort Text

- 0 → Cool
- 1 → Neutral
- 2 → Warm

## 6. Kết quả cuối cùng

Ví dụ serial output:

    Temp=29.3°C, Humidity=62% → Comfort: Neutral (prob=0.73)

MQTT có thể gửi:

    {
      "temperature": 29.3,
      "humidity": 62,
      "comfort_class": 1,
      "comfort_text": "Neutral",
      "probability": 0.73
    }
