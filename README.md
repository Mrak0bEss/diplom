# 🗂️ Lightweight C++ Cache Service with TTL Support

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/language-C++17-blue.svg)
![Crow Framework](https://img.shields.io/badge/web--framework-crow-lightgrey)

---

## 📚 Описание проекта

Этот проект реализует **легковесную систему кэширования** с возможностью:

- Генерации уникальных `GUID`;
- Хранения строковых значений по ключу `GUID`;
- Получения данных по ID;
- Поиска ID по содержимому;
- Задания **времени жизни (TTL)** для каждой записи;
- Автоматического удаления устаревших данных в фоне.

Сервер реализован на **C++** с использованием фреймворка **[Crow](https://github.com/CrowCpp/Crow)** — аналог Flask для C++.

---

## 🚀 Запуск проекта

### 🛠️ Сборка вручную

```bash
git clone https://github.com/your-name/cpp-cache-service.git
cd cpp-cache-service
mkdir build && cd build
cmake ..
make
./cache_server
```

Сервер стартует на порту **8080**.

### 🐳 Запуск через Docker

#### Шаги:

```bash
# Сборка Docker-образа
docker build -t cpp-cache .

# Запуск контейнера
docker run -p 8080:8080 cpp-cache
```

---

## 📬 REST API Методы

### 🔹 `GET /generate_guid`

🔧 **Назначение**: Сгенерировать новый уникальный `GUID`.

📤 **Ответ**:
```json
"0c2a4a7e-71ff-4e26-873f-65a3f6f5d0a1"
```

---

### 🔹 `POST /store/<guid>/<ttl>`

🔧 **Назначение**: Сохранить значение по ключу `GUID` с заданием времени жизни (TTL).

📥 **Параметры**:
- `<guid>` — ранее сгенерированный идентификатор;
- `<ttl>` — время хранения данных в **секундах**.

📦 **Тело запроса**:  
Текстовые данные, которые нужно сохранить.

📤 **Ответ**:  
Индекс (ID), под которым сохранены данные:
```json
"0"
```

---

### 🔹 `GET /retrieve/<guid>/<id>`

🔧 **Назначение**: Получить данные по `GUID` и `ID`.

📤 **Ответы**:
- `200 OK` — если данные актуальны:
```json
"some_value"
```
- `410 Gone` — если данные **устарели**:
```json
"Expired"
```

---

### 🔹 `POST /find/<guid>`

🔧 **Назначение**: Найти `ID` по значению.

📦 **Тело запроса**:  
Искомое значение.

📤 **Ответы**:
- `200 OK` — ID найден:
```json
"0"
```
- `404 Not Found` — значение не найдено;
- `410 Gone` — значение устарело.

---

## 🧹 Автоматическая очистка

⚙️ Каждые **5 секунд** запускается фоновый процесс, который удаляет устаревшие данные из кэша.

---

## 📁 Структура проекта

```
.
├── Dockerfile           # Контейнеризация
├── CMakeLists.txt       # Сборка CMake
├── main.cpp             # Основная логика кэша
└── README.md            # Описание проекта
```

---

## 🧪 Примеры запросов (cURL)

```bash
# Генерация GUID
curl http://localhost:8080/generate_guid

# Сохранение значения с TTL 60 сек
curl -X POST http://localhost:8080/store/<guid>/60 -d "hello world"

# Получение значения по ID
curl http://localhost:8080/retrieve/<guid>/0

# Поиск ID по значению
curl -X POST http://localhost:8080/find/<guid> -d "hello world"
```

---

## 👨‍💻 Зависимости

- `C++17`
- `Crow Framework`
- `Boost`
- `UUID (rpcrt4 on Windows)`
- `CMake`

---

## 📝 Лицензия

Проект распространяется под лицензией [MIT License](LICENSE).

---

## ✨ Автор

**ФИО** — Чубуков Андрей Владимирович
