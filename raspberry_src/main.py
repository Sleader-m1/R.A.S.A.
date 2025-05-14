import time
import RPi.GPIO as GPIO
from influxdb_client import InfluxDBClient

# Настройка подключения к InfluxDB
url = "http://localhost:8086"
token = "xZ3R5b2kKqAMvRwL6X3T4ZbKjHn7fPpYyNlGvC1zD8rRtS0Bm2E5QhI9JkOaU4XcW7Fd3V"
org = "rasa"
bucket = "rasa_sensor"

# Настройка GPIO
SERVO_PIN = 17
GPIO.setmode(GPIO.BCM)
GPIO.setup(SERVO_PIN, GPIO.OUT)
pwm = GPIO.PWM(SERVO_PIN, 50)
pwm.start(0)

# Функция для активации сервопривода
def activate_servo():
    pwm.ChangeDutyCycle(7.5)  # Угол 90°
    time.sleep(1)
    pwm.ChangeDutyCycle(0)

# Функция для получения последней температуры из InfluxDB
def get_latest_temperature():
    client = InfluxDBClient(url=url, token=token, org=org)
    query_api = client.query_api()
    query = f'''
    from(bucket: "{bucket}")
      |> range(start: -1m)
      |> filter(fn: (r) => r["_measurement"] == "temperature")
      |> filter(fn: (r) => r["_field"] == "value")
      |> last()
    '''
    result = query_api.query(query=query, org=org)
    for table in result:
        for record in table.records:
            return record.get_value()
    return None

try:
    while True:
        temperature = get_latest_temperature()
        if temperature is not None:
            print(f"Текущая температура: {temperature}°C")
            if temperature > 29:
                print("Температура превышена! Активируем сервопривод.")
                activate_servo()
        else:
            print("Не удалось получить температуру.")
        time.sleep(5)

except KeyboardInterrupt:
    print("Программа завершена.")
finally:
    pwm.stop()
    GPIO.cleanup()
