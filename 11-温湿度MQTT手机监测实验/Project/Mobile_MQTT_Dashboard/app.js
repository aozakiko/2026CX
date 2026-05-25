const STORAGE_KEY = "stm32-env-mobile-mqtt";

const defaults = {
  brokerUrl: "ws://192.168.166.177:9001/mqtt",
  clientId: `phone-env-${Math.random().toString(16).slice(2, 8)}`,
  envTopic: "stm32/env",
  temperatureTopic: "stm32/temperature",
  humidityTopic: "stm32/humidity",
  autoReconnect: true,
};

const elements = {
  state: document.querySelector("#connectionState"),
  temperature: document.querySelector("#temperatureValue"),
  humidity: document.querySelector("#humidityValue"),
  light: document.querySelector("#lightValue"),
  lux: document.querySelector("#luxValue"),
  lastUpdate: document.querySelector("#lastUpdateValue"),
  uptime: document.querySelector("#uptimeValue"),
  messageCount: document.querySelector("#messageCountValue"),
  form: document.querySelector("#settingsForm"),
  brokerUrl: document.querySelector("#brokerUrlInput"),
  clientId: document.querySelector("#clientIdInput"),
  envTopic: document.querySelector("#envTopicInput"),
  temperatureTopic: document.querySelector("#temperatureTopicInput"),
  humidityTopic: document.querySelector("#humidityTopicInput"),
  autoReconnect: document.querySelector("#autoReconnectInput"),
  connectButton: document.querySelector("#connectButton"),
  disconnectButton: document.querySelector("#disconnectButton"),
  activeTopic: document.querySelector("#activeTopicText"),
  messageLog: document.querySelector("#messageLog"),
};

const reading = {
  temperature: null,
  humidity: null,
  light: null,
  lux: null,
  uptime: null,
  messageCount: 0,
};

let client = null;

function readSettings() {
  try {
    return { ...defaults, ...JSON.parse(localStorage.getItem(STORAGE_KEY) || "{}") };
  } catch (error) {
    return { ...defaults };
  }
}

function writeSettings(settings) {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(settings));
}

function fillForm() {
  const settings = readSettings();
  elements.brokerUrl.value = settings.brokerUrl;
  elements.clientId.value = settings.clientId;
  elements.envTopic.value = settings.envTopic;
  elements.temperatureTopic.value = settings.temperatureTopic;
  elements.humidityTopic.value = settings.humidityTopic;
  elements.autoReconnect.checked = settings.autoReconnect;
  updateActiveTopicText(settings);
}

function collectForm() {
  return {
    brokerUrl: elements.brokerUrl.value.trim() || defaults.brokerUrl,
    clientId: elements.clientId.value.trim() || defaults.clientId,
    envTopic: elements.envTopic.value.trim() || defaults.envTopic,
    temperatureTopic: elements.temperatureTopic.value.trim() || defaults.temperatureTopic,
    humidityTopic: elements.humidityTopic.value.trim() || defaults.humidityTopic,
    autoReconnect: elements.autoReconnect.checked,
  };
}

function updateActiveTopicText(settings) {
  elements.activeTopic.textContent = [
    settings.envTopic,
    settings.temperatureTopic,
    settings.humidityTopic,
  ].join(" / ");
}

function setConnectionState(text, state) {
  elements.state.textContent = text;
  elements.state.dataset.state = state;
}

function pad(value) {
  return String(value).padStart(2, "0");
}

function formatClock(date) {
  return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
}

function formatUptime(seconds) {
  const total = Number(seconds);
  if (!Number.isFinite(total)) {
    return "--:--:--";
  }

  const h = Math.floor(total / 3600) % 24;
  const m = Math.floor((total % 3600) / 60);
  const s = Math.floor(total % 60);
  return `${pad(h)}:${pad(m)}:${pad(s)}`;
}

function numberFrom(value) {
  const num = Number(String(value).trim());
  return Number.isFinite(num) ? num : null;
}

function decodePayload(payload) {
  if (typeof payload === "string") {
    return payload;
  }

  return new TextDecoder().decode(payload);
}

function parseKeyValuePayload(payload) {
  const result = {};
  const parts = payload.trim().split(/[,;\s]+/);

  for (const part of parts) {
    const index = part.indexOf("=");
    if (index <= 0) {
      continue;
    }

    const key = part.slice(0, index).trim().toLowerCase();
    const value = part.slice(index + 1).trim();
    result[key] = value;
  }

  if (Object.keys(result).length === 0) {
    return null;
  }

  return {
    uptime: numberFrom(result.uptime || result.uptime_s),
    temperatureX100: numberFrom(result.temp_x100 || result.temperature_x100),
    temperature: numberFrom(result.temp || result.temperature),
    humidity: numberFrom(result.humidity),
    light: numberFrom(result.light || result.light_percent),
    lux: numberFrom(result.lux),
  };
}

function parseEnvFrame(payload) {
  const parts = payload.trim().split(",");
  if (parts[0] !== "ENV" || parts.length < 6) {
    return null;
  }

  return {
    uptime: numberFrom(parts[1]),
    temperatureX100: numberFrom(parts[2]),
    humidity: numberFrom(parts[3]),
    light: numberFrom(parts[4]),
    lux: numberFrom(parts[5]),
  };
}

function parseJsonPayload(payload) {
  try {
    const data = JSON.parse(payload);
    return {
      uptime: numberFrom(data.uptime ?? data.uptime_s),
      temperatureX100: numberFrom(data.temp_x100 ?? data.temperature_x100),
      temperature: numberFrom(data.temp ?? data.temperature),
      humidity: numberFrom(data.humidity),
      light: numberFrom(data.light ?? data.light_percent),
      lux: numberFrom(data.lux),
    };
  } catch (error) {
    return null;
  }
}

function normalizeReading(parsed) {
  if (!parsed) {
    return null;
  }

  const temperatureX100 = numberFrom(parsed.temperatureX100);
  let temperature = numberFrom(parsed.temperature);

  if (temperature === null && temperatureX100 !== null) {
    temperature = temperatureX100 / 100;
  }

  return {
    uptime: numberFrom(parsed.uptime),
    temperature,
    humidity: numberFrom(parsed.humidity),
    light: numberFrom(parsed.light),
    lux: numberFrom(parsed.lux),
  };
}

function consumeParsedReading(parsed) {
  const next = normalizeReading(parsed);
  if (!next) {
    return;
  }

  if (next.temperature !== null) {
    reading.temperature = next.temperature;
  }

  if (next.humidity !== null) {
    reading.humidity = next.humidity >= 0 ? next.humidity : null;
  }

  if (next.light !== null) {
    reading.light = next.light;
  }

  if (next.lux !== null) {
    reading.lux = next.lux;
  }

  if (next.uptime !== null) {
    reading.uptime = next.uptime;
  }

  updateDashboard();
}

function updateDashboard() {
  elements.temperature.textContent = reading.temperature === null
    ? "--.-"
    : reading.temperature.toFixed(1);
  elements.humidity.textContent = reading.humidity === null
    ? "--"
    : String(Math.round(reading.humidity));
  elements.light.textContent = reading.light === null
    ? "--"
    : String(Math.round(reading.light));
  elements.lux.textContent = reading.lux === null
    ? "--"
    : String(Math.round(reading.lux));
  elements.uptime.textContent = formatUptime(reading.uptime);
  elements.lastUpdate.textContent = formatClock(new Date());
  elements.messageCount.textContent = String(reading.messageCount);
}

function appendMessage(topic, payload) {
  reading.messageCount += 1;
  const item = document.createElement("li");
  const time = document.createElement("time");
  const code = document.createElement("code");

  time.textContent = `${formatClock(new Date())} ${topic}`;
  code.textContent = payload;
  item.append(time, code);
  elements.messageLog.prepend(item);

  while (elements.messageLog.children.length > 20) {
    elements.messageLog.lastElementChild.remove();
  }
}

function handleMessage(topic, payload) {
  const text = decodePayload(payload).trim();
  const settings = collectForm();

  appendMessage(topic, text);

  if (topic === settings.temperatureTopic) {
    consumeParsedReading({ temperature: numberFrom(text) });
    return;
  }

  if (topic === settings.humidityTopic) {
    consumeParsedReading({ humidity: numberFrom(text) });
    return;
  }

  const parsed = parseJsonPayload(text) || parseKeyValuePayload(text) || parseEnvFrame(text);
  consumeParsedReading(parsed);
}

function setButtons(isConnected) {
  elements.connectButton.disabled = isConnected;
  elements.disconnectButton.disabled = !isConnected;
}

function subscribeTopics(settings) {
  const topics = Array.from(new Set([
    settings.envTopic,
    settings.temperatureTopic,
    settings.humidityTopic,
  ].filter(Boolean)));

  for (const topic of topics) {
    client.subscribe(topic, { qos: 0 });
  }
}

function connectMqtt(settings) {
  if (!window.mqtt) {
    setConnectionState("脚本未加载", "error");
    return;
  }

  if (client) {
    client.end(true);
  }

  setConnectionState("连接中", "connecting");
  setButtons(false);
  updateActiveTopicText(settings);
  writeSettings(settings);

  client = mqtt.connect(settings.brokerUrl, {
    clientId: settings.clientId,
    clean: true,
    keepalive: 30,
    reconnectPeriod: settings.autoReconnect ? 2000 : 0,
    connectTimeout: 8000,
  });

  client.on("connect", () => {
    setConnectionState("在线", "online");
    setButtons(true);
    subscribeTopics(settings);
  });

  client.on("reconnect", () => {
    setConnectionState("重连中", "connecting");
  });

  client.on("close", () => {
    if (client) {
      setConnectionState("离线", "offline");
      setButtons(false);
    }
  });

  client.on("error", (error) => {
    setConnectionState(error.message || "连接错误", "error");
  });

  client.on("message", handleMessage);
}

function disconnectMqtt() {
  if (!client) {
    return;
  }

  const closingClient = client;
  client = null;
  closingClient.end(true);
  setConnectionState("离线", "offline");
  setButtons(false);
}

elements.form.addEventListener("submit", (event) => {
  event.preventDefault();
  connectMqtt(collectForm());
});

elements.disconnectButton.addEventListener("click", disconnectMqtt);

fillForm();
setButtons(false);
updateDashboard();
