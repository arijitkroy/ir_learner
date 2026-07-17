#include "WebFiles.h"

const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IR Learner Web Portal</title>
    <link rel="stylesheet" href="style.css">
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&display=swap" rel="stylesheet">
</head>
<body>
    <div class="app-container">
        <!-- Sidebar Navigation -->
        <aside class="sidebar">
            <div class="brand">
                <div class="logo"></div>
                <h2>IR Remote Hub</h2>
            </div>
            <nav class="nav-menu">
                <a href="#dashboard" class="nav-item active" data-tab="dashboard">
                    <svg viewBox="0 0 24 24" width="20" height="20" stroke="currentColor" stroke-width="2" fill="none"><rect x="3" y="3" width="7" height="9" rx="1"/><rect x="14" y="3" width="7" height="5" rx="1"/><rect x="14" y="12" width="7" height="9" rx="1"/><rect x="3" y="16" width="7" height="5" rx="1"/></svg>
                    <span>Dashboard</span>
                </a>
                <a href="#library" class="nav-item" data-tab="library">
                    <svg viewBox="0 0 24 24" width="20" height="20" stroke="currentColor" stroke-width="2" fill="none"><path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20"></path><path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z"></path></svg>
                    <span>IR Library</span>
                </a>
                <a href="#learn" class="nav-item" data-tab="learn">
                    <svg viewBox="0 0 24 24" width="20" height="20" stroke="currentColor" stroke-width="2" fill="none"><circle cx="12" cy="12" r="10"></circle><circle cx="12" cy="12" r="3" fill="currentColor"></circle></svg>
                    <span>IR Learn</span>
                </a>
                <a href="#logs" class="nav-item" data-tab="logs">
                    <svg viewBox="0 0 24 24" width="20" height="20" stroke="currentColor" stroke-width="2" fill="none"><polyline points="4 17 10 11 14 15 20 9"></polyline><polyline points="14 9 20 9 20 15"></polyline></svg>
                    <span>System Logs</span>
                </a>
                <a href="#settings" class="nav-item" data-tab="settings">
                    <svg viewBox="0 0 24 24" width="20" height="20" stroke="currentColor" stroke-width="2" fill="none"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>
                    <span>Settings</span>
                </a>
            </nav>
            <div class="system-status">
                <div class="status-indicator">
                    <span class="pulse"></span>
                    <span id="connection-state">Connecting...</span>
                </div>
            </div>
        </aside>

        <!-- Main Content Area -->
        <main class="main-content">
            <header class="top-bar">
                <h1>IR Remote Hub</h1>
                <div class="device-badge">
                    <span class="badge">Arduino Uno R4</span>
                </div>
            </header>

            <!-- Dashboard Section -->
            <section id="tab-dashboard" class="tab-panel">
                <h2 class="section-title">Dashboard</h2>
                <div class="dashboard-grid">
                    <!-- Status Cards -->
                    <div class="card status-card">
                        <h3>Device Status</h3>
                        <div class="stat-row">
                            <span>Uptime:</span>
                            <span id="uptime-val" class="value">-</span>
                        </div>
                        <div class="stat-row">
                            <span>Free RAM:</span>
                            <span id="ram-val" class="value">-</span>
                        </div>
                        <div class="stat-row">
                            <span>Flash Usage:</span>
                            <span id="flash-val" class="value">-</span>
                        </div>
                        <div class="stat-row">
                            <span>IR Interrupts:</span>
                            <span id="interrupts-val" class="value">0</span>
                        </div>
                    </div>
                    <div class="card status-card">
                        <h3>Network</h3>
                        <div class="stat-row">
                            <span>SSID:</span>
                            <span id="ssid-val" class="value">-</span>
                        </div>
                        <div class="stat-row">
                            <span>IP Address:</span>
                            <span id="ip-val" class="value">-</span>
                        </div>
                        <div class="stat-row">
                            <span>Signal Strength:</span>
                            <span id="rssi-val" class="value">-</span>
                        </div>
                    </div>
                    <!-- Direct Send Card -->
                    <div class="card send-card">
                        <h3>Quick Transmit</h3>
                        <form id="quick-send-form">
                            <div class="form-group">
                                <label for="tx-protocol">Protocol</label>
                                <select id="tx-protocol" class="form-control">
                                    <option value="1">NEC</option>
                                    <option value="3">Samsung</option>
                                    <option value="2">Sony</option>
                                    <option value="4">RC5</option>
                                    <option value="5">RC6</option>
                                    <option value="6">Panasonic</option>
                                    <option value="7">LG</option>
                                    <option value="8">JVC</option>
                                </select>
                            </div>
                            <div class="form-row">
                                <div class="form-group">
                                    <label for="tx-address">Address</label>
                                    <input type="number" id="tx-address" class="form-control" placeholder="0" min="0" max="65535">
                                </div>
                                <div class="form-group">
                                    <label for="tx-command">Command</label>
                                    <input type="number" id="tx-command" class="form-control" placeholder="0" min="0" max="4294967295">
                                </div>
                            </div>
                            <div class="form-row">
                                <div class="form-group">
                                    <label for="tx-bits">Bits</label>
                                    <input type="number" id="tx-bits" class="form-control" placeholder="32" min="1" max="64">
                                </div>
                                <div class="form-group">
                                    <label for="tx-freq">Freq (Hz)</label>
                                    <input type="number" id="tx-freq" class="form-control" placeholder="38000">
                                </div>
                            </div>
                            <button type="submit" class="btn btn-primary btn-block">Transmit Signal</button>
                        </form>
                    </div>
                </div>
            </section>

            <!-- Library Section -->
            <section id="tab-library" class="tab-panel">
                <h2 class="section-title">IR Library</h2>
                <div class="library-controls">
                    <div class="search-box">
                        <input type="text" id="lib-search" placeholder="Search commands...">
                    </div>
                    <div class="sort-box">
                        <select id="lib-sort">
                            <option value="name-asc">Name (A-Z)</option>
                            <option value="name-desc">Name (Z-A)</option>
                            <option value="protocol">Protocol</option>
                        </select>
                    </div>
                </div>
                <div id="library-grid" class="library-grid">
                    <!-- Dynamic code cards will be loaded here -->
                </div>
                <div id="library-empty" class="empty-state hidden">
                    <p>No saved commands found. Head over to the "IR Learn" tab to capture your first code!</p>
                </div>
            </section>

            <!-- Learn Section -->
            <section id="tab-learn" class="tab-panel">
                <h2 class="section-title">IR Learn</h2>
                <div class="card learn-wizard-card">
                    <div class="wizard-header">
                        <h2>IR Learning Wizard</h2>
                        <p>Capture IR commands and save them to the device library.</p>
                    </div>
                    <div class="wizard-body">
                        <!-- Step 1: Start -->
                        <div id="learn-step-idle" class="wizard-step active">
                            <div class="wizard-graphics wizard-graphic-idle"></div>
                            <button id="btn-start-learn" class="btn btn-primary btn-large">Start Capture Wizard</button>
                        </div>
                        <!-- Step 2: Capture -->
                        <div id="learn-step-listening" class="wizard-step">
                            <div class="wizard-graphics wizard-graphic-listening">
                                <div class="radar-pulse"></div>
                            </div>
                            <p class="status-msg">Listening... Point your remote at the receiver and press a key.</p>
                            <div class="pulse-counter">Pulses Captured: <span id="pulse-count">0</span></div>
                            <button id="btn-cancel-learn" class="btn btn-secondary">Cancel</button>
                        </div>
                        <!-- Step 3: Result -->
                        <div id="learn-step-result" class="wizard-step">
                            <div class="wizard-graphics wizard-graphic-success"></div>
                            <h3>Signal Captured!</h3>
                            <div class="decoded-details">
                                <div class="detail-row"><span>Protocol:</span><strong id="res-proto">-</strong></div>
                                <div class="detail-row"><span>Address:</span><strong id="res-addr">-</strong></div>
                                <div class="detail-row"><span>Command:</span><strong id="res-cmd">-</strong></div>
                                <div class="detail-row"><span>Bits:</span><strong id="res-bits">-</strong></div>
                                <div class="detail-row"><span>Signal Quality:</span><strong id="res-quality">-</strong></div>
                            </div>
                            <form id="save-code-form" class="save-form">
                                <div class="form-group">
                                    <label for="save-name">Name</label>
                                    <input type="text" id="save-name" class="form-control" placeholder="e.g. TV Power" required maxlength="20">
                                </div>
                                <div class="form-actions">
                                    <button type="button" id="btn-discard-learn" class="btn btn-secondary">Discard</button>
                                    <button type="submit" class="btn btn-primary">Save to Library</button>
                                </div>
                            </form>
                        </div>
                    </div>
                </div>
            </section>

            <!-- Logs Section -->
            <section id="tab-logs" class="tab-panel">
                <h2 class="section-title">System Logs</h2>
                <div class="card logs-card">
                    <div class="terminal-header">
                        <h3>Device Serial Logs</h3>
                        <button id="btn-clear-logs" class="btn btn-sm btn-secondary">Clear Terminal</button>
                    </div>
                    <div id="log-terminal" class="terminal-body">
                        <!-- WebSocket logs output -->
                    </div>
                </div>
            </section>

            <!-- Settings Section -->
            <section id="tab-settings" class="tab-panel">
                <h2 class="section-title">Settings</h2>
                <div class="settings-grid">
                    <!-- WiFi Config -->
                    <div class="card">
                        <h3>WiFi Station settings</h3>
                        <form id="wifi-config-form">
                            <div class="form-group">
                                <label for="wifi-ssid">Network SSID</label>
                                <input type="text" id="wifi-ssid" class="form-control" placeholder="Enter network name" required maxlength="32">
                            </div>
                            <div class="form-group">
                                <label for="wifi-pass">Password</label>
                                <input type="password" id="wifi-pass" class="form-control" placeholder="Enter network password" maxlength="64">
                            </div>
                            <div class="form-group">
                                <label for="wifi-mdns">mDNS Name</label>
                                <input type="text" id="wifi-mdns" class="form-control" placeholder="irremote" required maxlength="20">
                                <small>Device will be reachable at http://&lt;name&gt;.local</small>
                            </div>
                            <div class="form-group">
                                <label>
                                    <input type="checkbox" id="wifi-use-static"> Enable Static IP
                                </label>
                            </div>
                            <div id="static-ip-fields" class="hidden">
                                <div class="form-group">
                                    <label for="wifi-ip">Static IP Address</label>
                                    <input type="text" id="wifi-ip" class="form-control" placeholder="192.168.1.200">
                                </div>
                                <div class="form-group">
                                    <label for="wifi-gateway">Gateway Address</label>
                                    <input type="text" id="wifi-gateway" class="form-control" placeholder="192.168.1.1">
                                </div>
                                <div class="form-group">
                                    <label for="wifi-subnet">Subnet Mask</label>
                                    <input type="text" id="wifi-subnet" class="form-control" placeholder="255.255.255.0">
                                </div>
                            </div>
                            <button type="submit" class="btn btn-primary">Save & Apply Settings</button>
                        </form>
                    </div>

                    <!-- System & Database -->
                    <div class="card">
                        <h3>System Utilities</h3>
                        <div class="system-actions">
                            <button id="btn-reboot" class="btn btn-secondary btn-block">Reboot Controller</button>
                            <button id="btn-factory" class="btn btn-danger btn-block">Factory Reset Device</button>
                        </div>
                        <h3 class="margin-top">Database Backup</h3>
                        <div class="system-actions">
                            <button id="btn-export" class="btn btn-secondary btn-block">Export Database (JSON)</button>
                            <div class="file-upload-wrapper">
                                <button class="btn btn-secondary btn-block">Import Database (JSON)</button>
                                <input type="file" id="btn-import-file" accept=".json">
                            </div>
                        </div>
                    </div>
                </div>
            </section>
        </main>
    </div>

    <!-- Toast Notifications -->
    <div id="toast-container" class="toast-container"></div>

    <!-- Confirmation Modal Dialog -->
    <div id="confirm-modal" class="modal-overlay hidden">
        <div class="modal-card">
            <h3 id="modal-title">Confirm Action</h3>
            <p id="modal-msg">Are you sure you want to perform this operation?</p>
            <div class="modal-actions">
                <button id="modal-cancel" class="btn btn-secondary">Cancel</button>
                <button id="modal-confirm" class="btn btn-danger">Confirm</button>
            </div>
        </div>
    </div>

    <script src="app.js"></script>
</body>
</html>
)rawhtml";

const char STYLE_CSS[] PROGMEM = R"rawcss(
:root {
    --bg-base: #0b0f19;
    --bg-surface: #151c2c;
    --bg-card: rgba(26, 35, 54, 0.65);
    --border-color: rgba(255, 255, 255, 0.08);
    --text-primary: #f8fafc;
    --text-secondary: #94a3b8;
    --primary-color: #6366f1;
    --primary-hover: #4f46e5;
    --danger-color: #ef4444;
    --danger-hover: #dc2626;
    --success-color: #10b981;
    --success-hover: #059669;
    --font-sans: 'Outfit', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    --shadow-md: 0 4px 20px -2px rgba(0, 0, 0, 0.3);
    --backdrop-blur: blur(12px);
}
* { box-sizing: border-box; margin: 0; padding: 0; }
body { background-color: var(--bg-base); color: var(--text-primary); font-family: var(--font-sans); min-height: 100vh; overflow-x: hidden; }
.app-container { display: flex; min-height: 100vh; }
html { scroll-behavior: smooth; }
.sidebar { width: 260px; background-color: var(--bg-surface); border-right: 1px solid var(--border-color); display: flex; flex-direction: column; padding: 24px; flex-shrink: 0; position: fixed; top: 0; left: 0; height: 100vh; z-index: 10; }
.brand { display: flex; align-items: center; gap: 12px; margin-bottom: 40px; }
.logo { width: 32px; height: 32px; background: linear-gradient(135deg, var(--primary-color), #a855f7); border-radius: 8px; position: relative; box-shadow: 0 0 15px rgba(99, 102, 241, 0.4); }
.logo::after { content: ''; position: absolute; width: 12px; height: 12px; background-color: #fff; border-radius: 50%; top: 10px; left: 10px; }
.brand h2 { font-size: 1.15rem; font-weight: 700; letter-spacing: -0.02em; }
.nav-menu { display: flex; flex-direction: column; gap: 8px; flex-grow: 1; }
.nav-item { display: flex; align-items: center; gap: 12px; color: var(--text-secondary); text-decoration: none; padding: 12px 16px; border-radius: 10px; font-weight: 500; transition: all 0.25s ease; }
.nav-item svg { transition: transform 0.25s ease; }
.nav-item:hover, .nav-item.active { color: var(--text-primary); background-color: rgba(99, 102, 241, 0.1); }
.nav-item.active { background-color: var(--primary-color); box-shadow: 0 4px 15px rgba(99, 102, 241, 0.3); }
.nav-item.active svg { transform: scale(1.1); }
.system-status { padding-top: 20px; border-top: 1px solid var(--border-color); }
.status-indicator { display: flex; align-items: center; gap: 10px; font-size: 0.85rem; color: var(--text-secondary); }
.pulse { width: 8px; height: 8px; border-radius: 50%; background-color: var(--success-color); box-shadow: 0 0 10px var(--success-color); animation: pulse-animation 2s infinite; }
@keyframes pulse-animation {
    0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(16, 185, 129, 0.7); }
    70% { transform: scale(1); box-shadow: 0 0 0 8px rgba(16, 185, 129, 0); }
    100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(16, 185, 129, 0); }
}
.main-content { flex-grow: 1; display: flex; flex-direction: column; padding: 40px; margin-left: 260px; }
.top-bar { display: flex; justify-content: space-between; align-items: center; margin-bottom: 32px; }
.top-bar h1 { font-size: 1.85rem; font-weight: 700; letter-spacing: -0.03em; }
.badge { background-color: rgba(255, 255, 255, 0.05); border: 1px solid var(--border-color); padding: 6px 14px; border-radius: 30px; font-size: 0.85rem; font-weight: 600; color: var(--text-secondary); }
.card { background: var(--bg-card); backdrop-filter: var(--backdrop-blur); border: 1px solid var(--border-color); border-radius: 16px; padding: 28px; box-shadow: var(--shadow-md); transition: transform 0.3s ease, box-shadow 0.3s ease; }
.card:hover { transform: translateY(-2px); box-shadow: 0 8px 30px rgba(0, 0, 0, 0.4); }
.dashboard-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 24px; }
.status-card { display: flex; flex-direction: column; gap: 16px; }
.status-card h3, .card h3 { font-size: 1.1rem; font-weight: 600; color: var(--text-secondary); margin-bottom: 8px; }
.stat-row { display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid rgba(255, 255, 255, 0.03); padding-bottom: 10px; }
.stat-row:last-child { border-bottom: none; padding-bottom: 0; }
.stat-row span { font-size: 0.95rem; color: var(--text-secondary); }
.stat-row .value { font-weight: 600; color: var(--text-primary); }
.form-group { margin-bottom: 18px; }
.form-row { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; }
label { display: block; font-size: 0.85rem; font-weight: 600; color: var(--text-secondary); margin-bottom: 6px; text-transform: uppercase; letter-spacing: 0.05em; }
.form-control { width: 100%; background-color: rgba(255, 255, 255, 0.04); border: 1px solid var(--border-color); border-radius: 8px; padding: 10px 14px; color: #fff; font-family: var(--font-sans); font-size: 0.95rem; transition: all 0.2s ease; }
.form-control:focus { outline: none; border-color: var(--primary-color); background-color: rgba(255, 255, 255, 0.07); box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.25); }
select.form-control option { background-color: var(--bg-surface); }
.btn { display: inline-flex; align-items: center; justify-content: center; font-family: var(--font-sans); font-weight: 600; font-size: 0.95rem; padding: 10px 20px; border-radius: 8px; border: none; cursor: pointer; transition: all 0.2s ease; text-decoration: none; gap: 8px; }
.btn-primary { background-color: var(--primary-color); color: #fff; }
.btn-primary:hover { background-color: var(--primary-hover); box-shadow: 0 0 15px rgba(99, 102, 241, 0.4); }
.btn-secondary { background-color: rgba(255, 255, 255, 0.06); color: var(--text-primary); border: 1px solid var(--border-color); }
.btn-secondary:hover { background-color: rgba(255, 255, 255, 0.1); }
.btn-danger { background-color: var(--danger-color); color: #fff; }
.btn-danger:hover { background-color: var(--danger-hover); box-shadow: 0 0 15px rgba(239, 68, 68, 0.4); }
.btn-block { width: 100%; }
.btn-large { padding: 16px 32px; font-size: 1.1rem; border-radius: 12px; }
.btn-sm { padding: 6px 12px; font-size: 0.8rem; }
.tab-panel { display: none; animation: fade-in 0.3s ease; }
.tab-panel.active { display: block; }
@keyframes fade-in {
    from { opacity: 0; transform: translateY(8px); }
    to { opacity: 1; transform: translateY(0); }
}
.library-controls { display: flex; justify-content: space-between; margin-bottom: 24px; gap: 16px; }
.search-box { flex-grow: 1; }
.search-box input { width: 100%; background-color: var(--bg-surface); border: 1px solid var(--border-color); padding: 12px 18px; border-radius: 10px; color: #fff; font-size: 0.95rem; }
.search-box input:focus { outline: none; border-color: var(--primary-color); box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.25); }
.sort-box select { background-color: var(--bg-surface); border: 1px solid var(--border-color); padding: 12px 18px; border-radius: 10px; color: #fff; font-size: 0.95rem; cursor: pointer; }
.library-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(280px, 1fr)); gap: 20px; }
.code-card { background: var(--bg-card); border: 1px solid var(--border-color); border-radius: 14px; padding: 20px; display: flex; flex-direction: column; gap: 12px; transition: all 0.2s ease; }
.code-card:hover { border-color: rgba(99, 102, 241, 0.4); transform: scale(1.01); }
.code-card-header { display: flex; justify-content: space-between; align-items: flex-start; }
.code-title { font-weight: 600; font-size: 1.05rem; word-break: break-all; }
.code-meta { font-size: 0.8rem; color: var(--text-secondary); line-height: 1.5; }
.code-meta strong { color: var(--text-primary); }
.code-actions { display: flex; gap: 8px; margin-top: auto; }
.code-actions button { flex-grow: 1; }
.empty-state { text-align: center; padding: 80px 20px; color: var(--text-secondary); font-size: 1.05rem; border: 2px dashed rgba(255, 255, 255, 0.05); border-radius: 16px; }
.learn-wizard-card { max-width: 600px; margin: 0 auto; text-align: center; }
.wizard-header { margin-bottom: 30px; }
.wizard-step { display: none; flex-direction: column; align-items: center; gap: 24px; animation: fade-in 0.3s ease; }
.wizard-step.active { display: flex; }
.wizard-graphics { width: 140px; height: 140px; border-radius: 50%; background-color: rgba(255, 255, 255, 0.02); border: 1px solid var(--border-color); position: relative; display: flex; align-items: center; justify-content: center; }
.wizard-graphic-idle::after { content: '?'; font-size: 4rem; font-weight: 600; color: var(--text-secondary); }
.wizard-graphic-listening { border-color: var(--primary-color); }
.wizard-graphic-listening::after { content: ''; width: 24px; height: 24px; background-color: var(--primary-color); border-radius: 50%; }
.radar-pulse { position: absolute; width: 100%; height: 100%; border-radius: 50%; border: 2px solid var(--primary-color); animation: radar-pulse-anim 1.5s infinite linear; }
@keyframes radar-pulse-anim { 0% { transform: scale(0.9); opacity: 1; } 100% { transform: scale(1.6); opacity: 0; } }
.wizard-graphic-success { border-color: var(--success-color); background-color: rgba(16, 185, 129, 0.1); }
.wizard-graphic-success::after { content: '✓'; font-size: 4.5rem; font-weight: 600; color: var(--success-color); }
.decoded-details { width: 100%; background-color: rgba(0, 0, 0, 0.2); border-radius: 12px; padding: 16px; display: flex; flex-direction: column; gap: 8px; text-align: left; }
.detail-row { display: flex; justify-content: space-between; font-size: 0.95rem; }
.detail-row span { color: var(--text-secondary); }
.save-form { width: 100%; margin-top: 10px; text-align: left; }
.form-actions { display: flex; gap: 12px; margin-top: 20px; }
.form-actions button { flex-grow: 1; }
.logs-card { display: flex; flex-direction: column; height: 550px; }
.terminal-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; }
.terminal-body { flex-grow: 1; background-color: #05070d; border: 1px solid var(--border-color); border-radius: 10px; padding: 18px; font-family: "Courier New", Courier, monospace; font-size: 0.88rem; color: #4ade80; overflow-y: auto; white-space: pre-wrap; word-break: break-all; box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.8); }
.settings-grid { display: grid; grid-template-columns: 2fr 1fr; gap: 24px; }
.system-actions { display: flex; flex-direction: column; gap: 12px; }
.file-upload-wrapper { position: relative; overflow: hidden; }
.file-upload-wrapper input[type="file"] { position: absolute; left: 0; top: 0; opacity: 0; width: 100%; height: 100%; cursor: pointer; }
.margin-top { margin-top: 30px; }
.toast-container { position: fixed; bottom: 24px; right: 24px; display: flex; flex-direction: column; gap: 10px; z-index: 1000; }
.toast { background-color: #1e293b; border: 1px solid var(--border-color); border-left: 4px solid var(--primary-color); border-radius: 8px; padding: 16px 20px; color: #fff; font-size: 0.9rem; font-weight: 500; box-shadow: 0 10px 15px -3px rgba(0,0,0,0.5); display: flex; align-items: center; gap: 12px; transform: translateX(100%); animation: toast-in 0.3s forwards cubic-bezier(0.16, 1, 0.3, 1); }
@keyframes toast-in { to { transform: translateX(0); } }
.toast-success { border-left-color: var(--success-color); }
.toast-danger { border-left-color: var(--danger-color); }
.modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background-color: rgba(5, 7, 13, 0.85); backdrop-filter: blur(4px); display: flex; align-items: center; justify-content: center; z-index: 900; opacity: 1; transition: opacity 0.2s ease; }
.modal-card { background-color: var(--bg-surface); border: 1px solid var(--border-color); border-radius: 16px; padding: 30px; width: 90%; max-width: 420px; box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.6); text-align: center; animation: modal-zoom 0.25s forwards cubic-bezier(0.34, 1.56, 0.64, 1); }
@keyframes modal-zoom { from { transform: scale(0.9); } to { transform: scale(1); } }
.modal-card h3 { font-size: 1.25rem; font-weight: 700; margin-bottom: 12px; }
.modal-card p { color: var(--text-secondary); font-size: 0.95rem; line-height: 1.5; margin-bottom: 24px; }
.modal-actions { display: flex; gap: 12px; }
.modal-actions button { flex-grow: 1; }
.tab-panel { display: block; padding-top: 24px; margin-bottom: 48px; border-top: 1px solid var(--border-color); }
.tab-panel:first-of-type { border-top: none; padding-top: 0; }
.section-title { font-size: 1.5rem; font-weight: 700; letter-spacing: -0.02em; margin-bottom: 24px; color: var(--text-primary); }
.hidden { display: none !important; }
@media (max-width: 900px) {
    .app-container { flex-direction: column; }
    .sidebar { width: 100%; border-right: none; border-bottom: 1px solid var(--border-color); position: static; height: auto; }
    .settings-grid { grid-template-columns: 1fr; }
    .main-content { padding: 24px; margin-left: 0; }
}
)rawcss";

const char APP_JS[] PROGMEM = R"rawjavascript(
document.addEventListener("DOMContentLoaded", () => {
    const state = {
        codes: [],
        status: { state: "IDLE", wifiAP: false },
        wsConnected: false,
        activeTab: "dashboard",
        ws: null
    };

    const elements = {
        navItems: document.querySelectorAll(".nav-item"),
        connectionState: document.getElementById("connection-state"),
        
        uptimeVal: document.getElementById("uptime-val"),
        ramVal: document.getElementById("ram-val"),
        flashVal: document.getElementById("flash-val"),
        interruptsVal: document.getElementById("interrupts-val"),
        ssidVal: document.getElementById("ssid-val"),
        ipVal: document.getElementById("ip-val"),
        rssiVal: document.getElementById("rssi-val"),
        quickSendForm: document.getElementById("quick-send-form"),

        libraryGrid: document.getElementById("library-grid"),
        libraryEmpty: document.getElementById("library-empty"),
        libSearch: document.getElementById("lib-search"),
        libSort: document.getElementById("lib-sort"),

        btnStartLearn: document.getElementById("btn-start-learn"),
        btnCancelLearn: document.getElementById("btn-cancel-learn"),
        btnDiscardLearn: document.getElementById("btn-discard-learn"),
        stepIdle: document.getElementById("learn-step-idle"),
        stepListening: document.getElementById("learn-step-listening"),
        stepResult: document.getElementById("learn-step-result"),
        pulseCount: document.getElementById("pulse-count"),
        resProto: document.getElementById("res-proto"),
        resAddr: document.getElementById("res-addr"),
        resCmd: document.getElementById("res-cmd"),
        resBits: document.getElementById("res-bits"),
        resQuality: document.getElementById("res-quality"),
        saveCodeForm: document.getElementById("save-code-form"),
        saveName: document.getElementById("save-name"),

        logTerminal: document.getElementById("log-terminal"),
        btnClearLogs: document.getElementById("btn-clear-logs"),

        wifiConfigForm: document.getElementById("wifi-config-form"),
        wifiSsid: document.getElementById("wifi-ssid"),
        wifiPass: document.getElementById("wifi-pass"),
        wifiMdns: document.getElementById("wifi-mdns"),
        wifiUseStatic: document.getElementById("wifi-use-static"),
        staticIpFields: document.getElementById("static-ip-fields"),
        wifiIp: document.getElementById("wifi-ip"),
        wifiGateway: document.getElementById("wifi-gateway"),
        wifiSubnet: document.getElementById("wifi-subnet"),
        btnReboot: document.getElementById("btn-reboot"),
        btnFactory: document.getElementById("btn-factory"),
        btnExport: document.getElementById("btn-export"),
        btnImportFile: document.getElementById("btn-import-file"),

        confirmModal: document.getElementById("confirm-modal"),
        modalTitle: document.getElementById("modal-title"),
        modalMsg: document.getElementById("modal-msg"),
        modalCancel: document.getElementById("modal-cancel"),
        modalConfirm: document.getElementById("modal-confirm"),
        toastContainer: document.getElementById("toast-container")
    };

    const PROTOCOL_NAMES = {
        0: "Raw/Unknown",
        1: "NEC",
        2: "Sony",
        3: "Samsung",
        4: "RC5",
        5: "RC6",
        6: "Panasonic",
        7: "LG",
        8: "JVC"
    };

    function scrollToSection(sectionId) {
        const target = document.getElementById("tab-" + sectionId);
        if (target) target.scrollIntoView({ behavior: "smooth", block: "start" });
        elements.navItems.forEach(item => {
            if (item.getAttribute("data-tab") === sectionId) item.classList.add("active");
            else item.classList.remove("active");
        });
    }

    elements.navItems.forEach(item => {
        item.addEventListener("click", (e) => {
            e.preventDefault();
            const sectionId = item.getAttribute("data-tab");
            scrollToSection(sectionId);
            window.location.hash = sectionId;
        });
    });

    if (window.location.hash) {
        const tabHash = window.location.hash.substring(1);
        const validTabs = ["dashboard", "library", "learn", "logs", "settings"];
        if (validTabs.includes(tabHash)) setTimeout(() => scrollToSection(tabHash), 300);
    }

    function showToast(msg, type = "info") {
        const toast = document.createElement("div");
        toast.className = `toast toast-${type}`;
        toast.innerHTML = `<span>${msg}</span>`;
        elements.toastContainer.appendChild(toast);
        setTimeout(() => {
            toast.style.animation = "toast-in 0.3s reverse forwards";
            setTimeout(() => toast.remove(), 300);
        }, 3500);
    }

    let activeConfirmCallback = null;
    function showConfirm(title, msg, onConfirm) {
        elements.modalTitle.textContent = title;
        elements.modalMsg.textContent = msg;
        elements.confirmModal.classList.remove("hidden");
        activeConfirmCallback = onConfirm;
    }

    elements.modalCancel.addEventListener("click", () => {
        elements.confirmModal.classList.add("hidden");
        activeConfirmCallback = null;
    });

    elements.modalConfirm.addEventListener("click", () => {
        elements.confirmModal.classList.add("hidden");
        if (activeConfirmCallback) activeConfirmCallback();
        activeConfirmCallback = null;
    });

    function connectWebSocket() {
        const host = window.location.host || "192.168.4.1";
        const wsUrl = `ws://${host}/ws`;
        appendTerminalLog("Connecting to WebSocket: " + wsUrl);
        state.ws = new WebSocket(wsUrl);

        state.ws.onopen = () => {
            state.wsConnected = true;
            elements.connectionState.textContent = "Connected";
            elements.connectionState.parentElement.querySelector(".pulse").style.backgroundColor = "var(--success-color)";
            appendTerminalLog("WebSocket Connected!");
        };

        state.ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (data.event === "log") {
                    appendTerminalLog(`[${data.level.toUpperCase()}] ${data.msg}`);
                } else if (data.event === "pulse") {
                    elements.pulseCount.textContent = data.count;
                } else if (data.event === "learn") {
                    handleCapturedSignal(data);
                }
            } catch (e) {
                appendTerminalLog("System: " + event.data);
            }
        };

        state.ws.onclose = () => {
            state.wsConnected = false;
            elements.connectionState.textContent = "Reconnecting...";
            elements.connectionState.parentElement.querySelector(".pulse").style.backgroundColor = "var(--danger-color)";
            appendTerminalLog("WebSocket Disconnected. Reconnecting in 3s...");
            setTimeout(connectWebSocket, 3000);
        };
    }

    function appendTerminalLog(msg) {
        const el = document.createElement("div");
        el.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
        elements.logTerminal.appendChild(el);
        elements.logTerminal.scrollTop = elements.logTerminal.scrollHeight;
    }

    elements.btnClearLogs.addEventListener("click", () => {
        elements.logTerminal.innerHTML = "";
    });

    async function apiRequest(method, endpoint, body = null) {
        const options = { method };
        if (body) {
            options.headers = { "Content-Type": "application/json" };
            options.body = JSON.stringify(body);
        }
        try {
            const response = await fetch(endpoint, options);
            const data = await response.json();
            if (!response.ok) throw new Error(data.msg || "HTTP Error");
            return data;
        } catch (e) {
            showToast(e.message, "danger");
            throw e;
        }
    }

    async function loadStatus() {
        try {
            const data = await apiRequest("GET", "/api/status");
            state.status = data;
            elements.uptimeVal.textContent = formatUptime(data.uptime);
            elements.ramVal.textContent = formatBytes(data.freeRam);
            elements.flashVal.textContent = formatBytes(data.flashUsage);
            elements.interruptsVal.textContent = data.interrupts || 0;
            elements.ssidVal.textContent = data.wifiSSID;
            elements.ipVal.textContent = data.ip;
            elements.rssiVal.textContent = data.wifiAP ? "AP Mode" : `${data.rssi} dBm`;
            
            if (!elements.wifiSsid.value && !data.wifiAP) {
                elements.wifiSsid.value = data.wifiSSID;
            }
        } catch (e) {}
    }

    function formatUptime(secs) {
        const d = Math.floor(secs / 86400);
        const h = Math.floor((secs % 86400) / 3600);
        const m = Math.floor((secs % 3600) / 60);
        const s = secs % 60;
        let res = "";
        if (d > 0) res += `${d}d `;
        if (d > 0 || h > 0) res += `${h}h `;
        if (d > 0 || h > 0 || m > 0) res += `${m}m `;
        res += `${s}s`;
        return res;
    }

    function formatBytes(bytes) {
        if (bytes < 1024) return bytes + " B";
        else if (bytes < 1048576) return (bytes / 1024).toFixed(1) + " KB";
        return (bytes / 1048576).toFixed(1) + " MB";
    }

    elements.quickSendForm.addEventListener("submit", async (e) => {
        e.preventDefault();
        const payload = {
            protocol: parseInt(document.getElementById("tx-protocol").value),
            address: parseInt(elements.txAddress.value || 0),
            command: parseInt(elements.txCommand.value || 0),
            bits: parseInt(elements.txBits.value || 32),
            frequency: parseInt(elements.txFreq.value || 38000),
            rawLength: 0
        };
        try {
            await apiRequest("POST", "/api/send", payload);
            showToast("Signal transmitted successfully!", "success");
        } catch (e) {}
    });

    elements.txAddress = document.getElementById("tx-address");
    elements.txCommand = document.getElementById("tx-command");
    elements.txBits = document.getElementById("tx-bits");
    elements.txFreq = document.getElementById("tx-freq");

    async function loadLibrary() {
        try {
            const data = await apiRequest("GET", "/api/codes");
            state.codes = data;
            renderLibrary();
        } catch (e) {}
    }

    function renderLibrary() {
        elements.libraryGrid.innerHTML = "";
        let filtered = [...state.codes];
        const search = elements.libSearch.value.trim().toLowerCase();
        if (search) {
            filtered = filtered.filter(c => c.name.toLowerCase().includes(search));
        }

        const sortBy = elements.libSort.value;
        filtered.sort((a, b) => {
            if (sortBy === "name-asc") return a.name.localeCompare(b.name);
            if (sortBy === "name-desc") return b.name.localeCompare(a.name);
            if (sortBy === "protocol") return a.protocol - b.protocol;
            return 0;
        });

        if (filtered.length === 0) {
            elements.libraryEmpty.classList.remove("hidden");
            return;
        }
        elements.libraryEmpty.classList.add("hidden");

        filtered.forEach(code => {
            const card = document.createElement("div");
            card.className = "code-card";
            const protocolName = PROTOCOL_NAMES[code.protocol] || "Unknown";
            card.innerHTML = `
                <div class="code-card-header">
                    <div class="code-title">${code.name}</div>
                    <span class="badge">${protocolName}</span>
                </div>
                <div class="code-meta">
                    <div>Addr: <strong>0x${code.address.toString(16).toUpperCase()} (${code.address})</strong></div>
                    <div>Cmd: <strong>0x${code.command.toString(16).toUpperCase()} (${code.command})</strong></div>
                    <div>Bits: <strong>${code.bits}</strong> | Freq: <strong>${code.frequency}Hz</strong></div>
                    ${code.rawLength > 0 ? `<div>Raw length: <strong>${code.rawLength} pulses</strong></div>` : ""}
                </div>
                <div class="code-actions">
                    <button class="btn btn-sm btn-primary btn-send" data-id="${code.id}">Send</button>
                    <button class="btn btn-sm btn-secondary btn-duplicate" data-id="${code.id}">Dup</button>
                    <button class="btn btn-sm btn-secondary btn-rename" data-id="${code.id}">Edit</button>
                    <button class="btn btn-sm btn-danger btn-delete" data-id="${code.id}">Del</button>
                </div>
            `;
            elements.libraryGrid.appendChild(card);
        });

        elements.libraryGrid.querySelectorAll(".btn-send").forEach(btn => {
            btn.addEventListener("click", async () => {
                const id = parseInt(btn.getAttribute("data-id"));
                try {
                    await apiRequest("POST", "/api/send", { id });
                    showToast("Signal replayed!", "success");
                } catch(e) {}
            });
        });

        elements.libraryGrid.querySelectorAll(".btn-duplicate").forEach(btn => {
            btn.addEventListener("click", async () => {
                const id = parseInt(btn.getAttribute("data-id"));
                try {
                    const backup = await apiRequest("POST", "/api/export");
                    const database = backup.database;
                    const code = database.find(c => c.id === id);
                    if (code) {
                        const duplicateCode = JSON.parse(JSON.stringify(code));
                        duplicateCode.id = 0;
                        duplicateCode.name = duplicateCode.name + " (Copy)";
                        await apiRequest("POST", "/api/import", [duplicateCode]);
                        showToast("Command duplicated!", "success");
                        loadLibrary();
                    }
                } catch (e) {
                    showToast("Failed to duplicate command", "danger");
                }
            });
        });

        elements.libraryGrid.querySelectorAll(".btn-rename").forEach(btn => {
            btn.addEventListener("click", () => {
                const id = parseInt(btn.getAttribute("data-id"));
                const code = state.codes.find(c => c.id === id);
                if (code) {
                    const newName = prompt("Enter new name for command:", code.name);
                    if (newName && newName.trim() !== "") {
                        apiRequest("POST", "/api/rename", { id, name: newName.trim() })
                            .then(() => {
                                showToast("Renamed successfully!", "success");
                                loadLibrary();
                            });
                    }
                }
            });
        });

        elements.libraryGrid.querySelectorAll(".btn-delete").forEach(btn => {
            btn.addEventListener("click", () => {
                const id = parseInt(btn.getAttribute("data-id"));
                const code = state.codes.find(c => c.id === id);
                if (code) {
                    showConfirm("Delete Command", `Are you sure you want to delete "${code.name}"?`, async () => {
                        try {
                            await apiRequest("POST", "/api/delete", { id });
                            showToast("Deleted command", "success");
                            loadLibrary();
                        } catch(e) {}
                    });
                }
            });
        });
    }

    elements.libSearch.addEventListener("input", renderLibrary);
    elements.libSort.addEventListener("change", renderLibrary);

    function setWizardStep(stepName) {
        elements.stepIdle.classList.remove("active");
        elements.stepListening.classList.remove("active");
        elements.stepResult.classList.remove("active");
        if (stepName === "idle") elements.stepIdle.classList.add("active");
        else if (stepName === "listening") elements.stepListening.classList.add("active");
        else if (stepName === "result") elements.stepResult.classList.add("active");
    }

    elements.btnStartLearn.addEventListener("click", async () => {
        try {
            await apiRequest("POST", "/api/learn/start");
            elements.pulseCount.textContent = "0";
            setWizardStep("listening");
            showToast("Receiver is now listening...", "info");
        } catch(e) {}
    });

    elements.btnCancelLearn.addEventListener("click", async () => {
        try {
            await apiRequest("POST", "/api/learn/stop");
            setWizardStep("idle");
            showToast("Learning cancelled", "info");
        } catch(e) {}
    });

    function handleCapturedSignal(data) {
        if (data.status === "success") {
            setWizardStep("result");
            elements.resProto.textContent = PROTOCOL_NAMES[data.protocol] || "Raw/Unknown";
            elements.resAddr.textContent = data.protocol === 0 ? "N/A" : `0x${data.address.toString(16).toUpperCase()} (${data.address})`;
            elements.resCmd.textContent = data.protocol === 0 ? "N/A" : `0x${data.command.toString(16).toUpperCase()} (${data.command})`;
            elements.resBits.textContent = data.bits;
            elements.resQuality.textContent = `${data.quality}%`;
            elements.saveName.value = "";
            showToast("IR Signal Captured!", "success");
        } else {
            setWizardStep("idle");
            showToast("Capture timed out or failed.", "danger");
        }
    }

    elements.btnDiscardLearn.addEventListener("click", async () => {
        try {
            await apiRequest("POST", "/api/learn/stop");
            setWizardStep("idle");
            showToast("Signal discarded", "info");
        } catch(e) {}
    });

    elements.saveCodeForm.addEventListener("submit", async (e) => {
        e.preventDefault();
        const name = elements.saveName.value.trim();
        if (!name) return;
        try {
            const cap = await apiRequest("GET", "/api/learn/status");
            const payload = {
                name: name,
                protocol: cap.protocol,
                address: cap.address,
                command: cap.command,
                bits: cap.bits,
                frequency: 38000,
                rawLength: cap.protocol === 0 ? cap.pulses : 0,
                raw: cap.protocol === 0 && cap.raw ? cap.raw : []
            };
            await apiRequest("POST", "/api/codes", payload);
            showToast("Command saved to library!", "success");
            setWizardStep("idle");
            loadLibrary();
        } catch(e) {}
    });

    elements.wifiUseStatic.addEventListener("change", () => {
        if (elements.wifiUseStatic.checked) elements.staticIpFields.classList.remove("hidden");
        else elements.staticIpFields.classList.add("hidden");
    });

    elements.wifiConfigForm.addEventListener("submit", async (e) => {
        e.preventDefault();
        const payload = {
            wifiConfigured: 1,
            ssid: elements.wifiSsid.value.trim(),
            password: elements.wifiPass.value,
            mdnsName: elements.wifiMdns.value.trim(),
            ip: elements.wifiUseStatic.checked ? parseIpString(elements.wifiIp.value) : [0,0,0,0],
            gateway: elements.wifiUseStatic.checked ? parseIpString(elements.wifiGateway.value) : [0,0,0,0],
            subnet: elements.wifiUseStatic.checked ? parseIpString(elements.wifiSubnet.value) : [255,255,255,0]
        };
        showConfirm("Apply WiFi Settings", "Applying settings will reboot the controller. Proceed?", async () => {
            try {
                await apiRequest("POST", "/api/wifi", payload);
                showToast("Settings applied. Rebooting...", "success");
            } catch(e) {}
        });
    });

    function parseIpString(ipStr) {
        if (!ipStr) return [0,0,0,0];
        const parts = ipStr.split(".").map(p => parseInt(p));
        if (parts.length !== 4 || parts.some(isNaN)) return [0,0,0,0];
        return parts;
    }

    elements.btnReboot.addEventListener("click", () => {
        showConfirm("Reboot Controller", "Are you sure you want to reboot the device?", async () => {
            try {
                await apiRequest("POST", "/api/reboot");
                showToast("Reboot command sent.", "success");
            } catch(e) {}
        });
    });

    elements.btnFactory.addEventListener("click", () => {
        showConfirm("Factory Reset", "This will wipe all configuration and stored IR codes. Proceed?", async () => {
            try {
                await apiRequest("POST", "/api/factoryreset");
                showToast("Factory resetting...", "success");
            } catch(e) {}
        });
    });

    elements.btnExport.addEventListener("click", async () => {
        try {
            const data = await apiRequest("POST", "/api/export");
            const blob = new Blob([JSON.stringify(data.database, null, 2)], { type: "application/json" });
            const url = URL.createObjectURL(blob);
            const a = document.createElement("a");
            a.href = url;
            a.download = `ir_backup_${Date.now()}.json`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
            showToast("Database exported!", "success");
        } catch(e) {}
    });

    elements.btnImportFile.addEventListener("change", async (e) => {
        const file = e.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = async (evt) => {
            try {
                const parsed = JSON.parse(evt.target.result);
                await apiRequest("POST", "/api/import", parsed);
                showToast("Database imported successfully!", "success");
                loadLibrary();
            } catch (err) {
                showToast("Failed to import: invalid JSON structure", "danger");
            }
        };
        reader.readAsText(file);
    });

    loadStatus();
    loadLibrary();
    setInterval(loadStatus, 4000);
    connectWebSocket();
});
)rawjavascript";
