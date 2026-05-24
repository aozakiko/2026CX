const connectBtn = document.querySelector("#connectBtn");
const statusText = document.querySelector("#statusText");
const uptimeText = document.querySelector("#uptimeText");
const dateText = document.querySelector("#dateText");
const timeText = document.querySelector("#timeText");
const tempValue = document.querySelector("#tempValue");
const humidityValue = document.querySelector("#humidityValue");
const lightValue = document.querySelector("#lightValue");

let port;
let reader;
let keepReading = false;

function pad(value) {
  return String(value).padStart(2, "0");
}

function formatUptime(seconds) {
  const total = Number(seconds) || 0;
  const h = Math.floor(total / 3600) % 24;
  const m = Math.floor((total % 3600) / 60);
  const s = total % 60;
  return `${pad(h)}:${pad(m)}:${pad(s)}`;
}

function tickClock() {
  const now = new Date();
  const weekdays = ["星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"];
  dateText.textContent = `${now.getFullYear()}年${now.getMonth() + 1}月${now.getDate()}日${weekdays[now.getDay()]}`;
  timeText.textContent = `${pad(now.getHours())}:${pad(now.getMinutes())}:${pad(now.getSeconds())}`;
}

function parseEnvLine(line) {
  const parts = line.trim().split(",");
  if (parts[0] !== "ENV" || parts.length < 6) {
    return null;
  }

  return {
    uptime: Number(parts[1]),
    tempX100: Number(parts[2]),
    humidity: Number(parts[3]),
    lightPercent: Number(parts[4]),
    lux: Number(parts[5]),
  };
}

function updateDashboard(data) {
  if (!Number.isNaN(data.tempX100)) {
    tempValue.textContent = (data.tempX100 / 100).toFixed(1);
  }

  humidityValue.textContent = data.humidity >= 0 ? String(data.humidity) : "--";

  if (!Number.isNaN(data.lux)) {
    lightValue.textContent = String(data.lux);
  }

  uptimeText.textContent = `STM32 时间 ${formatUptime(data.uptime)}`;
  statusText.textContent = data.humidity >= 0
    ? `已接收 温度 ${(data.tempX100 / 100).toFixed(1)}C / 湿度 ${data.humidity}% / 光照 ${data.lightPercent}%`
    : `已接收 温度 ${(data.tempX100 / 100).toFixed(1)}C / 湿度 -- / 光照 ${data.lightPercent}%`;
}

async function readSerial() {
  const decoder = new TextDecoder();
  let buffer = "";

  while (port?.readable && keepReading) {
    reader = port.readable.getReader();
    try {
      while (true) {
        const { value, done } = await reader.read();
        if (done) {
          break;
        }

        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split(/\r?\n/);
        buffer = lines.pop() ?? "";

        for (const line of lines) {
          const data = parseEnvLine(line);
          if (data) {
            updateDashboard(data);
          }
        }
      }
    } finally {
      reader.releaseLock();
    }
  }
}

async function connectSerial() {
  if (!("serial" in navigator)) {
    statusText.textContent = "当前浏览器不支持 Web Serial，请用 Chrome 或 Edge 打开";
    return;
  }

  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200, dataBits: 8, stopBits: 1, parity: "none", flowControl: "none" });
    keepReading = true;
    connectBtn.textContent = "断开";
    statusText.textContent = "串口已连接";
    readSerial();
  } catch (error) {
    statusText.textContent = `连接失败：${error.message}`;
  }
}

async function disconnectSerial() {
  keepReading = false;
  try {
    await reader?.cancel();
    await port?.close();
  } finally {
    port = undefined;
    reader = undefined;
    connectBtn.textContent = "连接串口";
    statusText.textContent = "未连接";
  }
}

connectBtn.addEventListener("click", () => {
  if (port) {
    disconnectSerial();
  } else {
    connectSerial();
  }
});

tickClock();
setInterval(tickClock, 1000);
