(() => {
  const BUILD_TAG = '2026-04-20';
  const STORAGE_KEY = 'suminagashi.settings.v2';
  const QUALITY_LEVELS = [
    { id: 0, name: 'Performance', scale: 0.75 },
    { id: 1, name: 'Balanced', scale: 0.9 },
    { id: 2, name: 'High', scale: 1.0 }
  ];

  const PRESETS = {
    balanced: { label: 'Balanced Flow', paletteIndex: 1, qualityMode: 1, radius: 80, strength: 80, sharpness: 32, mode: 'drops' },
    sunset: { label: 'Sunset Wash', paletteIndex: 0, qualityMode: 2, radius: 96, strength: 112, sharpness: 44, mode: 'drops' },
    calm: { label: 'Calm Marble', paletteIndex: 15, qualityMode: 0, radius: 64, strength: 58, sharpness: 24, mode: 'drops' },
    contrast: { label: 'Contrast Ripple', paletteIndex: 6, qualityMode: 1, radius: 120, strength: 140, sharpness: 64, mode: 'tine' }
  };

  const PATTERN_BLOOM_NAMES = [
    'Golden Spiral Bloom',
    'Lissajous Weave',
    'Petal Rings',
    'Harmonic Lattice',
    'Ribbon Galaxy'
  ];

  const state = {
    runtimeReady: false,
    mode: 'drops',
    qualityMode: 1,
    paletteIndex: 0,
    selectedPaletteColorIndex: 0,
    layoutPending: false,
    layoutReason: 'boot',
    settings: {
      radius: 80,
      strength: 80,
      sharpness: 32,
      exportPrefix: 'suminagashi',
      exportTimestamp: true,
      preset: 'balanced',
      qualityMode: 1,
      paletteIndex: 0,
      selectedPaletteColorIndex: 0,
      mode: 'drops'
    },
    metrics: {
      cssWidth: 0,
      cssHeight: 0,
      renderWidth: 0,
      renderHeight: 0,
      dpr: 1,
      qualityScale: 0.9
    }
  };

  const el = {};

  function byId(id) {
    return document.getElementById(id);
  }

  function hasNativeExport(name) {
    return typeof Module !== 'undefined' && typeof Module[`_${name}`] === 'function';
  }

  function callNative(name, returnType = 'void', argTypes = [], args = []) {
    if (typeof Module === 'undefined' || typeof Module.ccall !== 'function') {
      return null;
    }

    if (!hasNativeExport(name)) {
      return null;
    }

    try {
      return Module.ccall(name, returnType, argTypes, args);
    } catch (error) {
      console.error(`[native] ${name} failed`, error);
      return null;
    }
  }

  function nextFrame() {
    return new Promise(resolve => requestAnimationFrame(() => resolve()));
  }

  function delayFrames(count) {
    return (async () => {
      for (let i = 0; i < count; i += 1) {
        await nextFrame();
      }
    })();
  }

  function formatSize(width, height) {
    return `${width}x${height}`;
  }

  function getQualityLevel(mode) {
    return QUALITY_LEVELS.find(item => item.id === mode) || QUALITY_LEVELS[1];
  }

  function normalizeMode(mode) {
    if (mode === 'random') {
      return 'randomPalette';
    }
    return mode === 'tine' || mode === 'randomPalette' || mode === 'randomFull' ? mode : 'drops';
  }

  function log(message, payload) {
    if (payload === undefined) {
      console.info(`[runtime] ${message}`);
    } else {
      console.info(`[runtime] ${message}`, payload);
    }
  }

  function showToast(message, type = 'success') {
    const toast = el.toast;
    const toastMessage = el.toastMessage;
    const icon = el.toastIcon;

    if (!toast || !toastMessage || !icon) {
      console.warn(message);
      return;
    }

    toastMessage.textContent = message;
    toast.className = `toast ${type}`;
    icon.className = type === 'error' ? 'fas fa-times' : 'fas fa-check';
    toast.classList.add('show');

    window.clearTimeout(state.toastTimer);
    state.toastTimer = window.setTimeout(() => {
      toast.classList.remove('show');
    }, 2800);
  }

  function readSettings() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY);
      if (!raw) {
        return;
      }

      const parsed = JSON.parse(raw);
      state.settings = { ...state.settings, ...parsed };
      state.mode = normalizeMode(state.settings.mode);
      state.qualityMode = Number.isFinite(state.settings.qualityMode) ? state.settings.qualityMode : 1;
      state.paletteIndex = Number.isFinite(state.settings.paletteIndex) ? state.settings.paletteIndex : 0;
      state.selectedPaletteColorIndex = Number.isFinite(state.settings.selectedPaletteColorIndex)
        ? state.settings.selectedPaletteColorIndex
        : 0;
    } catch (error) {
      console.warn('[runtime] failed to read settings', error);
    }
  }

  function writeSettings() {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(state.settings));
    } catch (error) {
      console.warn('[runtime] failed to persist settings', error);
    }
  }

  function syncSettingsFromUi() {
    state.settings.radius = Number.parseInt(el.radiusSlider?.value ?? state.settings.radius, 10);
    state.settings.strength = Number.parseInt(el.strengthSlider?.value ?? state.settings.strength, 10);
    state.settings.sharpness = Number.parseInt(el.sharpnessSlider?.value ?? state.settings.sharpness, 10);
    state.settings.exportPrefix = el.exportPrefix?.value?.trim() || 'suminagashi';
    state.settings.exportTimestamp = Boolean(el.exportTimestamp?.checked);
    state.settings.preset = el.presetSelect?.value || state.settings.preset;
    state.settings.qualityMode = state.qualityMode;
    state.settings.paletteIndex = state.paletteIndex;
    state.settings.selectedPaletteColorIndex = state.selectedPaletteColorIndex;
    state.settings.mode = state.mode;
  }

  function applySettingsToUi() {
    if (el.radiusSlider) el.radiusSlider.value = String(state.settings.radius);
    if (el.strengthSlider) el.strengthSlider.value = String(state.settings.strength);
    if (el.sharpnessSlider) el.sharpnessSlider.value = String(state.settings.sharpness);
    if (el.exportPrefix) el.exportPrefix.value = state.settings.exportPrefix;
    if (el.exportTimestamp) el.exportTimestamp.checked = Boolean(state.settings.exportTimestamp);
    if (el.presetSelect) el.presetSelect.value = state.settings.preset;
    setQualityMode(state.settings.qualityMode, false);
    setMode(state.settings.mode, false);
    updateSliderLabels();
  }

  function buildFilename() {
    const prefix = el.exportPrefix?.value?.trim() || 'suminagashi';
    const timestampEnabled = Boolean(el.exportTimestamp?.checked);
    if (!timestampEnabled) {
      return `${prefix}.png`;
    }

    const stamp = new Date().toISOString().replace(/[:.]/g, '-').slice(0, 19);
    return `${prefix}-${stamp}.png`;
  }

  function updateMetricsPanel() {
    if (el.metricCss) el.metricCss.textContent = formatSize(state.metrics.cssWidth, state.metrics.cssHeight);
    if (el.metricRender) el.metricRender.textContent = formatSize(state.metrics.renderWidth, state.metrics.renderHeight);
    if (el.metricDpr) el.metricDpr.textContent = state.metrics.dpr.toFixed(2);
    if (el.metricQuality) el.metricQuality.textContent = getQualityLevel(state.qualityMode).name;
    if (el.metricMode) {
      el.metricMode.textContent = state.mode === 'tine'
        ? 'Tine'
        : state.mode === 'randomPalette'
          ? 'Random Palette'
          : state.mode === 'randomFull'
            ? 'Random Full'
            : 'Drops';
    }
  }

  function updateSliderLabels() {
    if (el.strengthValue) el.strengthValue.textContent = String(el.strengthSlider?.value ?? '0');
    if (el.sharpnessValue) el.sharpnessValue.textContent = String(el.sharpnessSlider?.value ?? '0');
    if (el.radiusValue) el.radiusValue.textContent = String(el.radiusSlider?.value ?? '0');
  }

  function setQualityMode(mode, persist = true) {
    const nextMode = getQualityLevel(Number(mode)).id;
    state.qualityMode = nextMode;

    if (el.qualityButtons) {
      [...el.qualityButtons.querySelectorAll('button')].forEach(button => {
        button.classList.toggle('btn-primary', Number(button.dataset.mode) === nextMode);
      });
    }

    if (persist) {
      syncSettingsFromUi();
      writeSettings();
    }

    callNative('setQualityMode', null, ['number'], [nextMode]);
    scheduleLayoutSync('quality');
    updateMetricsPanel();
  }

  function setMode(mode, persist = true) {
    state.mode = normalizeMode(mode);

    if (el.modeDrops) el.modeDrops.classList.toggle('btn-primary', state.mode === 'drops');
    if (el.modeRandomPalette) el.modeRandomPalette.classList.toggle('btn-primary', state.mode === 'randomPalette');
    if (el.modeRandomFull) el.modeRandomFull.classList.toggle('btn-primary', state.mode === 'randomFull');
    if (el.modeTine) el.modeTine.classList.toggle('btn-primary', state.mode === 'tine');

    const nativeMode = state.mode === 'tine' ? 1 : state.mode === 'randomPalette' ? 2 : state.mode === 'randomFull' ? 3 : 0;
    callNative('setInteractionMode', null, ['number'], [nativeMode]);

    if (persist) {
      syncSettingsFromUi();
      writeSettings();
    }

    updateMetricsPanel();
    updateStatusText();
  }

  function updateStatusText() {
    if (!el.statusText) {
      return;
    }

    el.statusText.textContent = state.mode === 'tine'
      ? 'Tine mode: click to carve'
      : state.mode === 'randomPalette'
        ? 'Random palette mode: click for random palette color and random radius'
        : state.mode === 'randomFull'
          ? 'Random full mode: click for random color and random radius'
        : 'Ready to create';
  }

  function decodePackedColor(packed) {
    return {
      r: (packed >>> 24) & 0xff,
      g: (packed >>> 16) & 0xff,
      b: (packed >>> 8) & 0xff,
      a: packed & 0xff
    };
  }

  function applySelectedPaletteColor() {
    if (!hasNativeExport('getCurrentPaletteSize') || !hasNativeExport('getCurrentPaletteColor')) {
      return;
    }

    const count = callNative('getCurrentPaletteSize', 'number', [], []) || 0;
    if (count <= 0) {
      state.selectedPaletteColorIndex = 0;
      return;
    }

    if (state.selectedPaletteColorIndex >= count || state.selectedPaletteColorIndex < 0) {
      state.selectedPaletteColorIndex = 0;
    }

    const packed = callNative('getCurrentPaletteColor', 'number', ['number'], [state.selectedPaletteColorIndex]);
    const rgba = decodePackedColor(packed);
    callNative('setNextDropColor', null, ['number', 'number', 'number', 'number'], [rgba.r, rgba.g, rgba.b, rgba.a]);
  }

  function rebuildPaletteSelect() {
    if (!el.paletteSelect) {
      return;
    }

    if (!hasNativeExport('getPaletteCount')) {
      el.paletteSelect.innerHTML = '';
      return;
    }

    const count = callNative('getPaletteCount', 'number', [], []) || 0;
    if (count <= 0) {
      el.paletteSelect.innerHTML = '';
      state.paletteIndex = 0;
      return;
    }

    if (state.paletteIndex < 0 || state.paletteIndex >= count) {
      state.paletteIndex = 0;
    }

    el.paletteSelect.innerHTML = '';
    for (let index = 0; index < count; index += 1) {
      const option = document.createElement('option');
      option.value = String(index);
      option.textContent = `Palette ${index + 1}`;
      el.paletteSelect.appendChild(option);
    }

    el.paletteSelect.value = String(state.paletteIndex);
  }

  function rebuildPaletteButtons() {
    if (!el.paletteColors) {
      return;
    }

    if (!hasNativeExport('getCurrentPaletteSize') || !hasNativeExport('getCurrentPaletteColor')) {
      el.paletteColors.innerHTML = '';
      return;
    }

    el.paletteColors.innerHTML = '';
    const count = callNative('getCurrentPaletteSize', 'number', [], []) || 0;

    if (count > 0 && (state.selectedPaletteColorIndex >= count || state.selectedPaletteColorIndex < 0)) {
      state.selectedPaletteColorIndex = 0;
    }

    for (let index = 0; index < count; index += 1) {
      const packed = callNative('getCurrentPaletteColor', 'number', ['number'], [index]);
      const rgba = decodePackedColor(packed);
      const button = document.createElement('button');
      button.type = 'button';
      button.className = 'palette-swatch';
      button.style.background = `rgba(${rgba.r}, ${rgba.g}, ${rgba.b}, ${rgba.a / 255})`;
      button.title = `RGB(${rgba.r}, ${rgba.g}, ${rgba.b})`;
      button.addEventListener('click', () => {
        state.selectedPaletteColorIndex = index;
        applySelectedPaletteColor();
        rebuildPaletteButtons();
        syncSettingsFromUi();
        writeSettings();
      });
      if (index === state.selectedPaletteColorIndex) {
        button.classList.add('selected');
      }
      el.paletteColors.appendChild(button);
    }

    if (count > 0 && state.selectedPaletteColorIndex >= count) {
      state.selectedPaletteColorIndex = 0;
    }

    applySelectedPaletteColor();
  }

  function applyPreset(presetId) {
    const preset = PRESETS[presetId] || PRESETS.balanced;
    state.settings.preset = presetId in PRESETS ? presetId : 'balanced';
    state.paletteIndex = preset.paletteIndex;
    state.settings.radius = preset.radius;
    state.settings.strength = preset.strength;
    state.settings.sharpness = preset.sharpness;
    state.selectedPaletteColorIndex = 0;
    state.mode = normalizeMode(preset.mode);
    state.qualityMode = preset.qualityMode;

    if (el.presetSelect) {
      el.presetSelect.value = state.settings.preset;
    }

    if (el.radiusSlider) el.radiusSlider.value = String(preset.radius);
    if (el.strengthSlider) el.strengthSlider.value = String(preset.strength);
    if (el.sharpnessSlider) el.sharpnessSlider.value = String(preset.sharpness);

    setMode(preset.mode, false);
    setQualityMode(preset.qualityMode, false);
    callNative('setPaletteIndex', null, ['number'], [preset.paletteIndex]);
    rebuildPaletteSelect();
    callNative('setNextDropRadius', null, ['number'], [preset.radius]);
    rebuildPaletteButtons();
    updateSliderLabels();
    syncSettingsFromUi();
    writeSettings();
  }

  function getCanvasRect() {
    const canvas = el.canvas;
    if (!canvas) {
      return { width: 0, height: 0 };
    }

    const rect = canvas.getBoundingClientRect();
    if (rect.width > 0 && rect.height > 0) {
      return { width: Math.round(rect.width), height: Math.round(rect.height) };
    }

    const container = el.canvasContainer;
    const fallbackWidth = Math.max(1, container?.clientWidth || 1200);
    return { width: fallbackWidth, height: Math.max(1, Math.round(fallbackWidth * 2 / 3)) };
  }

  function syncLayout(reason = 'resize') {
    if (!el.canvas) {
      return;
    }

    const rect = getCanvasRect();
    const dpr = Math.max(1, window.devicePixelRatio || 1);
    const quality = getQualityLevel(state.qualityMode);

    state.metrics.cssWidth = rect.width;
    state.metrics.cssHeight = rect.height;
    state.metrics.dpr = dpr;
    state.metrics.qualityScale = quality.scale;
    state.metrics.renderWidth = Math.max(1, Math.round(rect.width * dpr * quality.scale));
    state.metrics.renderHeight = Math.max(1, Math.round(rect.height * dpr * quality.scale));

    if (state.runtimeReady) {
      callNative('syncCanvasViewport', null, ['number', 'number', 'number', 'number'], [rect.width, rect.height, dpr, quality.scale]);
    }

    updateMetricsPanel();
    log(`layout sync (${reason})`, { ...state.metrics });
  }

  function scheduleLayoutSync(reason = 'resize') {
    state.layoutReason = reason;
    if (state.layoutPending) {
      return;
    }

    state.layoutPending = true;
    requestAnimationFrame(() => {
      requestAnimationFrame(() => {
        state.layoutPending = false;
        syncLayout(state.layoutReason);
      });
    });
  }

  async function saveScreenshot() {
    const canvas = el.canvas;
    if (!canvas) {
      showToast('Canvas is not ready', 'error');
      return;
    }

    try {
      await delayFrames(2);
      scheduleLayoutSync('screenshot');
      await delayFrames(1);

      const blob = await new Promise((resolve, reject) => {
        if (typeof canvas.toBlob !== 'function') {
          reject(new Error('toBlob unavailable'));
          return;
        }

        canvas.toBlob(result => {
          if (!result) {
            reject(new Error('canvas returned an empty blob'));
            return;
          }
          resolve(result);
        }, 'image/png');
      });

      const filename = buildFilename();
      const url = URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.href = url;
      link.download = filename;
      document.body.appendChild(link);
      link.click();
      link.remove();
      setTimeout(() => URL.revokeObjectURL(url), 1000);
      showToast(`Saved ${filename}`, 'success');
      log('screenshot saved', { filename, metrics: { ...state.metrics } });
    } catch (error) {
      console.error('[screenshot] export failed', error);
      showToast('Screenshot export failed', 'error');
    }
  }

  function clearCanvas() {
    if (callNative('clearCanvas', null, [], []) === null && !hasNativeExport('clearCanvas')) {
      showToast('Canvas controls are not ready', 'error');
      return;
    }

    showToast('Canvas cleared', 'success');
    scheduleLayoutSync('clear');
  }

  function fitCanvas() {
    scheduleLayoutSync('fit');
    showToast('Canvas layout synced', 'success');
  }

  function resetView() {
    setMode('drops', false);
    applyPreset(state.settings.preset || 'balanced');
    scheduleLayoutSync('reset');
    showToast('View reset', 'success');
  }

  function triggerPatternBloom() {
    if (!hasNativeExport('generatePatternBloom')) {
      showToast('Pattern Bloom is not ready', 'error');
      return;
    }

    const patternId = callNative('generatePatternBloom', 'number', [], []);
    if (hasNativeExport('getCurrentPaletteIndex')) {
      const paletteIndex = callNative('getCurrentPaletteIndex', 'number', [], []);
      if (Number.isFinite(paletteIndex) && paletteIndex >= 0) {
        state.paletteIndex = paletteIndex;
      }
    }

    state.selectedPaletteColorIndex = 0;
    rebuildPaletteSelect();
    rebuildPaletteButtons();
    syncSettingsFromUi();
    writeSettings();
    scheduleLayoutSync('pattern-bloom');

    const safePatternId = Number.isFinite(patternId) ? patternId : 0;
    const patternName = PATTERN_BLOOM_NAMES[((safePatternId % PATTERN_BLOOM_NAMES.length) + PATTERN_BLOOM_NAMES.length) % PATTERN_BLOOM_NAMES.length];
    showToast(`Pattern Bloom: ${patternName}`, 'success');
  }

  function toggleFullscreen() {
    const canvas = el.canvas;
    if (!canvas) {
      showToast('Canvas is not ready', 'error');
      return;
    }

    if (!document.fullscreenElement) {
      canvas.requestFullscreen().then(() => scheduleLayoutSync('fullscreen')).catch(() => {
        showToast('Fullscreen not supported', 'error');
      });
    } else {
      document.exitFullscreen().then(() => scheduleLayoutSync('fullscreen-exit')).catch(() => {
        showToast('Fullscreen exit failed', 'error');
      });
    }
  }

  function showInfo() {
    el.instructions?.scrollIntoView({ behavior: 'smooth', block: 'start' });
  }

  function showAbout() {
    window.alert('Suminagashi - Digital Paper Marbling\n\nBuilt with C++, Raylib, and WebAssembly.');
  }

  function attachEventHandlers() {
    el.modeDrops?.addEventListener('click', () => setMode('drops'));
    el.modeRandomPalette?.addEventListener('click', () => setMode('randomPalette'));
    el.modeRandomFull?.addEventListener('click', () => setMode('randomFull'));
    el.modeTine?.addEventListener('click', () => setMode('tine'));
    el.clearButton?.addEventListener('click', clearCanvas);
    el.screenshotButton?.addEventListener('click', () => saveScreenshot());
    el.fullscreenButton?.addEventListener('click', toggleFullscreen);
    el.infoButton?.addEventListener('click', showInfo);
    el.resetViewButton?.addEventListener('click', resetView);
    el.fitCanvasButton?.addEventListener('click', fitCanvas);
    el.patternBloomButton?.addEventListener('click', triggerPatternBloom);
    el.radiusSlider?.addEventListener('input', () => {
      updateSliderLabels();
      callNative('setNextDropRadius', null, ['number'], [Number.parseInt(el.radiusSlider.value, 10)]);
      syncSettingsFromUi();
      writeSettings();
    });
    el.strengthSlider?.addEventListener('input', () => {
      updateSliderLabels();
      syncSettingsFromUi();
      writeSettings();
    });
    el.sharpnessSlider?.addEventListener('input', () => {
      updateSliderLabels();
      syncSettingsFromUi();
      writeSettings();
    });
    el.exportPrefix?.addEventListener('input', () => {
      syncSettingsFromUi();
      writeSettings();
    });
    el.exportTimestamp?.addEventListener('change', () => {
      syncSettingsFromUi();
      writeSettings();
    });
    el.presetSelect?.addEventListener('change', () => applyPreset(el.presetSelect.value));
    el.paletteSelect?.addEventListener('change', () => {
      const nextIndex = Number.parseInt(el.paletteSelect.value, 10);
      state.paletteIndex = Number.isFinite(nextIndex) ? nextIndex : 0;
      state.selectedPaletteColorIndex = 0;
      callNative('setPaletteIndex', null, ['number'], [state.paletteIndex]);
      rebuildPaletteButtons();
      syncSettingsFromUi();
      writeSettings();
    });
    el.qualityButtons?.querySelectorAll('button').forEach(button => {
      button.addEventListener('click', () => setQualityMode(Number(button.dataset.mode)));
    });
    el.paletteColors?.addEventListener('contextmenu', event => event.preventDefault());

    window.addEventListener('resize', () => scheduleLayoutSync('window-resize'));
    window.addEventListener('orientationchange', () => scheduleLayoutSync('orientationchange'));
    window.addEventListener('load', () => scheduleLayoutSync('load'));
    document.addEventListener('visibilitychange', () => {
      if (!document.hidden) {
        scheduleLayoutSync('visibility');
      }
    });

    if (el.canvasContainer && 'ResizeObserver' in window) {
      const observer = new ResizeObserver(() => scheduleLayoutSync('observer'));
      observer.observe(el.canvasContainer);
      state.resizeObserver = observer;
    }

    if (el.canvas) {
      el.canvas.addEventListener('contextmenu', event => event.preventDefault());
      let pointerDown = false;
      let lastX = -9999;
      const minDelta = 4;

      function invokeTine(clientX) {
        if (state.mode !== 'tine') {
          return;
        }

        const rect = el.canvas.getBoundingClientRect();
        const canvasX = (clientX - rect.left) * (el.canvas.width / rect.width);
        const strength = Number.parseFloat(el.strengthSlider?.value || '0');
        const sharpness = Number.parseFloat(el.sharpnessSlider?.value || '1');
        callNative('applyTineAt', null, ['number', 'number', 'number'], [canvasX, strength, sharpness]);
      }

      el.canvas.addEventListener('mousedown', event => {
        if (event.button !== 0) {
          return;
        }

        pointerDown = true;
        lastX = -9999;
        invokeTine(event.clientX);
      });
      window.addEventListener('mouseup', () => { pointerDown = false; });
      el.canvas.addEventListener('mousemove', event => {
        if (!pointerDown) {
          return;
        }

        const rect = el.canvas.getBoundingClientRect();
        const x = (event.clientX - rect.left) * (el.canvas.width / rect.width);
        if (Math.abs(x - lastX) >= minDelta) {
          invokeTine(event.clientX);
          lastX = x;
        }
      });
      el.canvas.addEventListener('mouseleave', () => { pointerDown = false; });
    }

    window.addEventListener('keydown', event => {
      if (event.key === 't' || event.key === 'T') {
        setMode(state.mode === 'drops' ? 'tine' : 'drops');
      }
    });
  }

  function attachNativeHooks() {
    const previousRuntimeInit = Module.onRuntimeInitialized;
    Module.onRuntimeInitialized = () => {
      if (typeof previousRuntimeInit === 'function') {
        previousRuntimeInit();
      }

      state.runtimeReady = true;
      hideLoading();
      rebuildPaletteSelect();
      rebuildPaletteButtons();
      syncSettingsFromUi();
      applySettingsToRuntime();
      scheduleLayoutSync('runtime-init');
      showToast('Suminagashi loaded', 'success');
      log('runtime initialized', { buildTag: BUILD_TAG });
    };
  }

  function hideLoading() {
    if (el.loading) {
      el.loading.style.display = 'none';
    }
  }

  function applySettingsToRuntime() {
    callNative('setNextDropRadius', null, ['number'], [Number.parseInt(el.radiusSlider?.value || '80', 10)]);
    callNative('setPaletteIndex', null, ['number'], [state.paletteIndex]);
    callNative('setInteractionMode', null, ['number'], [state.mode === 'tine' ? 1 : state.mode === 'randomPalette' ? 2 : state.mode === 'randomFull' ? 3 : 0]);
    callNative('setQualityMode', null, ['number'], [state.qualityMode]);
    rebuildPaletteSelect();
    rebuildPaletteButtons();
    updateMetricsPanel();
  }

  function bindElements() {
    el.canvas = byId('canvas');
    el.canvasContainer = byId('canvas-container');
    el.loading = byId('loading');
    el.toast = byId('toast');
    el.toastMessage = byId('toast-message');
    el.toastIcon = el.toast?.querySelector('i') || null;
    el.modeDrops = byId('modeDrops');
    el.modeRandomPalette = byId('modeRandomPalette');
    el.modeRandomFull = byId('modeRandomFull');
    el.modeTine = byId('modeTine');
    el.clearButton = byId('clearButton');
    el.screenshotButton = byId('screenshotButton');
    el.fullscreenButton = byId('fullscreenButton');
    el.infoButton = byId('infoButton');
    el.resetViewButton = byId('resetViewButton');
    el.fitCanvasButton = byId('fitCanvasButton');
    el.patternBloomButton = byId('patternBloomButton');
    el.strengthSlider = byId('strengthSlider');
    el.sharpnessSlider = byId('sharpnessSlider');
    el.radiusSlider = byId('radiusSlider');
    el.strengthValue = byId('strengthVal');
    el.sharpnessValue = byId('sharpnessVal');
    el.radiusValue = byId('radiusVal');
    el.paletteSelect = byId('paletteSelect');
    el.paletteColors = byId('paletteColors');
    el.qualityButtons = byId('qualityButtons');
    el.presetSelect = byId('presetSelect');
    el.exportPrefix = byId('exportPrefix');
    el.exportTimestamp = byId('exportTimestamp');
    el.metricCss = byId('metricCss');
    el.metricRender = byId('metricRender');
    el.metricDpr = byId('metricDpr');
    el.metricQuality = byId('metricQuality');
    el.metricMode = byId('metricMode');
    el.statusText = byId('statusText');
    el.instructions = byId('instructions');
  }

  function configureModule() {
    if (typeof Module === 'undefined') {
      window.Module = {};
    }

    const previousLocateFile = Module.locateFile;
    Module.locateFile = (path, prefix) => {
      if (path.endsWith('.wasm')) {
        return `suminasashi.wasm?v=${BUILD_TAG}`;
      }
      if (typeof previousLocateFile === 'function') {
        return previousLocateFile(path, prefix);
      }
      return path;
    };

    attachNativeHooks();
  }

  function initialize() {
    bindElements();
    readSettings();
    configureModule();
    attachEventHandlers();
    applySettingsToUi();
    syncLayout('boot');
    updateMetricsPanel();
    updateStatusText();
    showToast('Loading Suminagashi...', 'success');
  }

  function exposeGlobals() {
    window.clearCanvas = clearCanvas;
    window.saveScreenshot = saveScreenshot;
    window.toggleFullscreen = toggleFullscreen;
    window.showInfo = showInfo;
    window.showAbout = showAbout;
    window.setMode = setMode;
    window.fitCanvas = fitCanvas;
    window.resetView = resetView;
    window.triggerPatternBloom = triggerPatternBloom;
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
      initialize();
      exposeGlobals();
    }, { once: true });
  } else {
    initialize();
    exposeGlobals();
  }
})();