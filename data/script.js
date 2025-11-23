// ========== SIDEBAR & LAYOUT ==========

(function () {
    const sidebar = document.getElementById('sidebar');
    const toggle = document.getElementById('sidebarToggle');
    const overlay = document.getElementById('sidebar-overlay') || document.querySelector('.sidebar-overlay');

    // If toggle exists -> wire collapse behavior
    if (toggle && sidebar) {
        toggle.addEventListener('click', () => {
            // collapse / expand
            sidebar.classList.toggle('sidebar-collapsed');
            // rotate chevron icon accordingly
            const icon = toggle.querySelector('.material-icons-outlined');
            if (icon) icon.textContent = sidebar.classList.contains('sidebar-collapsed') ? 'chevron_right' : 'chevron_left';
        });
    }

    // Activate correct menu item based on path or data-page
    const current = window.location.pathname.split('/').pop() || 'index.html';
    document.querySelectorAll('.sidebar-list-item').forEach(li => {
        const page = li.getAttribute('data-page') || (li.querySelector('a') && li.querySelector('a').getAttribute('href'));
        if (page && (current === page || (current === '' && page === 'dashboard.html'))) {
            li.classList.add('active');
        } else li.classList.remove('active');

        // click navigation behavior (if you use layout fetchs)
        li.addEventListener('click', () => {
            // if the list items have data-page and you're using layout loader, call loadPage
            const pageTo = li.getAttribute('data-page');
            if (pageTo && typeof loadPage === 'function') {
                loadPage(pageTo);
            } else if (pageTo) {
                // fallback: navigate to page
                window.location.href = pageTo;
            }
        });
    });

    // Mobile overlay open/close (optional)
    document.querySelectorAll('.menu-icon, .header .material-icons-outlined').forEach(btn => {
        btn && btn.addEventListener('click', () => {
            if (!sidebar) return;
            sidebar.classList.toggle('sidebar-open');
            overlay && overlay.classList.toggle('active');
        });
    });
    overlay && overlay.addEventListener('click', () => {
        if (!sidebar) return;
        sidebar.classList.remove('sidebar-open');
        overlay.classList.remove('active');
    });
})();


// ========== FAKE CARD DATA (WIND / RAIN) ==========
let tempChart = null;
let humChart = null;
function initDynamicCharts() {
    const tempCanvas = document.getElementById("tempChart");
    const humCanvas = document.getElementById("humidityChart");

    if (!tempCanvas || !humCanvas || typeof Chart === "undefined") return;

    const tempCtx = tempCanvas.getContext("2d");
    const humCtx = humCanvas.getContext("2d");

    // Nhiệt độ
    tempChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Temperature (°C)',
                data: [],
                borderColor: 'rgb(255, 99, 132)',
                backgroundColor: 'rgba(255, 99, 132, 0.3)',
                pointBackgroundColor: 'rgb(255, 99, 132)',
                fill: true,
                tension: 0.4
            }]
        },
        options: getChartOptions()
    });

    // Độ ẩm
    humChart = new Chart(humCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Humidity (%)',
                data: [],
                borderColor: 'rgb(54, 162, 235)',
                backgroundColor: 'rgba(54, 162, 235, 0.3)',
                pointBackgroundColor: 'rgb(54, 162, 235)',
                fill: true,
                tension: 0.4
            }]
        },
        options: getChartOptions()
    });

    setInterval(updateCharts, 3000);
}


function getChartOptions() {
    return {
        responsive: true,
        animation: false,
        scales: {
            x: {
                ticks: { color: '#fff' },
                grid: { color: '#444' },
            },
            y: {
                beginAtZero: true,
                ticks: { color: '#fff' },
                grid: { color: '#444' }
            }
        },
        plugins: {
            legend: {
                labels: { color: '#fff' }
            }
        }
    };
}

let lastTimeLabel = "";

function updateCharts() {
    if (!tempChart || !humChart) return;

    fetch("/sensors")
        .then(res => res.json())
        .then(data => {
            const temp = parseFloat(data.temp);
            const hum = parseFloat(data.hum);

            if (isNaN(temp) || isNaN(hum)) return;

            // tạo label thời gian
            const now = new Date();
            const label = now.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit', second: '2-digit' });

            appendData(tempChart, label, temp);
            appendData(humChart, label, hum);

            // cập nhật card
            document.getElementById("temperature").textContent = temp.toFixed(1) + " °C";
            document.getElementById("humidity").textContent = hum.toFixed(1) + " %";
        })
        .catch(err => console.error("updateCharts error:", err));
}


// Làm tròn giờ hiện tại về mốc 5 phút gần nhất, trả về chuỗi HH:mm
function getRoundedTimeLabel(date) {
    const minutes = date.getMinutes();
    const roundedMinutes = Math.floor(minutes / 5) * 5;
    const hours = date.getHours();

    const hh = hours.toString().padStart(2, "0");
    const mm = roundedMinutes.toString().padStart(2, "0");

    return `${hh}:${mm}`;
}

function appendData(chart, label, value) {
    chart.data.labels.push(label);
    chart.data.datasets[0].data.push(value);

    if (chart.data.labels.length > 10) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
    }

    chart.update();
}

function updateEnvFromSensors() {
    const tempEl = document.getElementById("temperature");
    const humEl = document.getElementById("humidity");

    if (!tempEl || !humEl) return;

    fetch("/sensors")
        .then(res => res.json())
        .then(data => {
            if (typeof data.temp === "number") {
                tempEl.textContent = data.temp.toFixed(1) + " °C";
            }
            if (typeof data.hum === "number") {
                humEl.textContent = data.hum.toFixed(1) + " %";
            }
        })
        .catch(err => {
            console.error("Error fetching /sensors:", err);
        });
}

// Khi dashboard đã load xong (index + dashboard.html đã chèn vào DOM)
// window.addEventListener("load", () => {
//     // gọi lần đầu
//     updateEnvFromSensors();
//     initDynamicCharts();
//     // cập nhật lại mỗi 3 giây
//     setInterval(updateEnvFromSensors, 3000);
// });

function updateDeviceUI(data) {
    // === LED 1 & LED 2 ===
    const devices = ['led1', 'led2', 'pump', 'fan'];

    devices.forEach(device => {
        const statusElement = document.getElementById(device + 'Status');
        const iconWrapper = document.getElementById(device + 'Icon');
        const dataKey = device; // Tên key trong JSON API: data.led1, data.pump, data.fan

        if (data[dataKey] && statusElement && iconWrapper) {
            const status = data[dataKey];
            statusElement.innerText = status;

            if (status === "ON") {
                iconWrapper.classList.add("led-on");
                iconWrapper.classList.remove("led-off");
            } else {
                iconWrapper.classList.add("led-off");
                iconWrapper.classList.remove("led-on");
            }
        }
    });
}

// device: 'led1', 'led2', 'pump', hoặc 'fan', state: 'on' | 'off'
function toggleDevice(device, state) {
    let ledId = 0;

    // Nếu là LED, dùng ID số (1 hoặc 2) như API cũ của bạn
    if (device === 'led1') ledId = 1;
    else if (device === 'led2') ledId = 2;
    // Nếu là Fan/Pump, dùng tên thiết bị (string)
    else if (device === 'pump') ledId = 'pump';
    else if (device === 'fan') ledId = 'fan';
    else return; // Thiết bị không xác định

    fetch(`/set?led=${ledId}&state=${state}`)
        .then(res => res.json())
        .then(data => {
            console.log("SET response:", data);
            // Sử dụng hàm cập nhật UI mới
            updateDeviceUI(data);
        })
        .catch(err => console.error("Device control error:", err));
}

function setNeoColor(color) {
    fetch(`/neopixel?hex=${encodeURIComponent(color)}`)
        .then(res => res.text())
        .then(text => {
            console.log("NEOPIXEL response:", text);
            const neoStatus = document.getElementById("neoStatus");
            if (neoStatus) {
                neoStatus.innerText = `Color: ${color.toUpperCase()}`;
            }
        })
        .catch(err => console.error("NeoPixel Error:", err));
}

function allOn() {
    fetch('/set_all?state=on')
        .then(res => res.json())
        .then(data => {
            console.log("ALL ON response:", data);
            // Hàm updateDeviceUI sẽ cập nhật tất cả (LED, Pump, Fan) nếu API /set_all trả về status của tất cả.
            updateDeviceUI(data);
        })
        .catch(err => console.error("All ON error:", err));
}

function allOff() {
    fetch('/set_all?state=off')
        .then(res => res.json())
        .then(data => {
            console.log("ALL OFF response:", data);
            // Hàm updateDeviceUI sẽ cập nhật tất cả (LED, Pump, Fan)
            updateDeviceUI(data);
        })
        .catch(err => console.error("All OFF error:", err));
}


// ================== WIFI CONFIG PAGE ==================
document.addEventListener("DOMContentLoaded", () => {
    const form = document.getElementById("wifiForm");
    if (!form) return;           // nếu không phải wifi.html thì thôi

    const btn = document.getElementById("btnConnect");
    const msg = document.getElementById("msg");
    const toast = document.getElementById("toast");
    const togglePass = document.getElementById("togglePass");
    const passInput = document.getElementById("pass");

    // Hiện / ẩn mật khẩu
    if (togglePass && passInput) {
        togglePass.onclick = () => {
            passInput.type = (passInput.type === "password") ? "text" : "password";
        };
    }

    form.onsubmit = (e) => {
        e.preventDefault();

        const ssid = document.getElementById("ssid").value.trim();
        const pass = passInput.value;

        if (!ssid) {
            msg.textContent = "Vui lòng nhập SSID.";
            return;
        }
        if (pass.length < 8) {
            msg.textContent = "Mật khẩu tối thiểu 8 ký tự.";
            return;
        }

        const oldHTML = btn.innerHTML;
        btn.disabled = true;
        btn.innerHTML = '<span class="spinner"></span>Connecting...';
        msg.textContent = "";

        // Gửi yêu cầu connect tới ESP32
        fetch('/connect?ssid=' + encodeURIComponent(ssid) +
            '&pass=' + encodeURIComponent(pass))
            .then(r => r.text())
            .then(() => {
                // báo đang kết nối
                toast.className = 'toast show';
                toast.textContent = 'Đang kết nối Wi-Fi...';

                // check /wifi_status mỗi 1s
                let check = setInterval(() => {
                    fetch('/wifi_status')
                        .then(r => r.text())
                        .then(st => {
                            console.log('wifi_status =', st);

                            if (st === 'connected') {
                                clearInterval(check);
                                toast.textContent = 'Kết nối thành công!';
                                setTimeout(() => {
                                    // ✅ chỉ khi connected mới chuyển trang
                                    window.location.href = '/';
                                }, 1200);
                            } else if (st === 'failed') {
                                clearInterval(check);
                                toast.className = 'toast error show';
                                toast.textContent = 'Kết nối thất bại. Kiểm tra SSID/mật khẩu.';

                                btn.disabled = false;
                                btn.innerHTML = oldHTML;
                            }
                            // st === 'connecting' → cứ để đó, không redirect
                        })
                        .catch(err => {
                            console.error("wifi_status error:", err);
                        });
                }, 1000);
            })
            .catch(err => {
                console.error("connect error:", err);
                toast.className = 'toast error show';
                toast.textContent = 'Không gửi được yêu cầu đến thiết bị.';

                btn.disabled = false;
                btn.innerHTML = oldHTML;
            });
    };
});
// =======================================================
// ========== LOGIC POLLING TRẠNG THÁI VÀ KHỞI ĐỘNG ==========
// =======================================================

/**
 * Hàm Polling: Lấy trạng thái hiện tại từ ESP32 qua API /state.
 * Hàm này tận dụng updateDeviceUI đã có sẵn.
 */
function pollDeviceState() {
    fetch('/state')
        .then(res => {
            if (!res.ok) {
                console.error("Failed to fetch /state:", res.status);
                throw new Error('Network response was not ok');
            }
            return res.json();
        })
        .then(data => {
            // Sử dụng hàm đã có để cập nhật giao diện
            updateDeviceUI(data);
        })
        .catch(error => console.error('Polling state error:', error));
}


// Thiết lập Khởi động và Lặp lại (Chạy sau khi DOM load xong)
document.addEventListener("DOMContentLoaded", () => {
    // ... (logic WIFI CONFIG PAGE hiện có của bạn) ...

    const form = document.getElementById("wifiForm");

    // Thêm logic khởi động cho trang điều khiển
    if (window.location.pathname.includes('control.html') ||
        (window.location.pathname === '/' && !form)) {
        // 1. Lấy trạng thái ban đầu khi tải trang
        pollDeviceState();

        // 2. Thiết lập Polling lặp lại mỗi 1 giây (1000ms)
        setInterval(pollDeviceState, 1000);
    }
});