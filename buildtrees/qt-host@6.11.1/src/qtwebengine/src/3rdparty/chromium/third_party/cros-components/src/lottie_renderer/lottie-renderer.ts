/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';

// TODO: b/295990177 - Make these absolute.
import {waitForEvent} from '../async_helpers/async_helpers';
import {assertExists} from '../helpers/helpers';

import {addColorSchemeChangeListener, removeColorSchemeChangeListener} from './event_binders';
import {ProcessedLottieJSON} from './lottie_processing_helpers';
import {defaultGetWorker} from './worker';

/** The CustomEvent names that LottieRenderer can fire. */
export enum CrosLottieEvent {
  /**
   * Fired when the animation has been loaded on the worker thread and is
   * ready to play.
   */
  INITIALIZED = `cros-lottie-initialized`,
  /**
   * Fired when the animation has been paused on the worker thread.
   */
  PAUSED = `cros-lottie-paused`,
  /**
   * Fired when the animation has begun playing on the worker thread.
   */
  PLAYING = `cros-lottie-playing`,
  /**
   * Fired when the animation has been resized on the worker thread.
   */
  RESIZED = `cros-lottie-resized`,
  /**
   * Fired when the animation has begun playing on the worker thread.
   */
  STOPPED = `cros-lottie-stopped`,
}

/**
 * A object or class with methods cros-lottie-renderer can use to log warnings
 * and errors with lottie file processing and playing.
 */
export interface LottieRendererLogger {
  warn(message: string): void;
  error(message: string): void;
}

/**
 * The reason cros-lottie-renderer initalized a animation. Can be either because
 * of being rendered for the first time, because the color scheme changed or
 * because the animation changes.
 */
export type DataMessageReason =
    'first-run'|'colors-changed'|'asset-url-changed'|'unknown';

/**
 * Event fired by cros-lottie-renderer when the web worker has successfully
 * loaded an animation file.
 */
export type CrosLottieInitializedEvent = CustomEvent<DataMessageReason>;

interface ResizeDetail {
  width: number;
  height: number;
}

declare interface MessageData {
  animationData: object;
  drawSize: {width: number, height: number};
  params: {loop: boolean, subframeEnabled: boolean, autoplay: boolean};
  canvas?: OffscreenCanvas;
}

/**
 * Lottie renderer with correct hooks to handle dynamic color. This component
 * should only be used for dynamic illustrations, as it involves spinning up an
 * instance of the Lottie library on a worker thread. For static illustrations,
 * it is more performant to use <cros-static-illustration>.
 */
export class LottieRenderer extends LitElement {
  /**
   * The path to the lottie asset JSON file.
   * @export
   */
  declare assetUrl: string;

  /**
   * When true, animation will begin to play as soon as it's loaded.
   * @export
   */
  declare autoplay: boolean;

  /**
   * When true, animation will loop continuously.
   * @export
   */
  declare loop: boolean;

  /**
   * Whether or not the illustration should render using a dynamic palette
   * taken from the computed style, or the default GM2 palette each asset comes
   * with.
   * @export
   */
  declare dynamic: boolean;

  /**
   * Function which returns a web worker loaded up with lottie worker js.
   * If unspecified in chromium defaults to a standard web worker pulling from
   * "chrome://resources/cros_components/lottie_renderer/lottie_worker.js". If
   * unspecified in google3 defaults to a standard web worker pulling from
   * "/js/lottie_worker.js".
   * Must be set before attaching lottieRenderer to the DOM:
   *   const renderer = new LottieRenderer();
   *   renderer.getWorker = myCustomGetWorker();
   *   document.body.append(renderer);
   * @export
   */
  getWorker: () => Worker = defaultGetWorker;

  /**
   * A object that lottie-renderer can use to log warnings and errors with
   * attempting to run a lottie file. If unspecified defaults to dumping
   * logs into the console.
   * Note lottie renderer only logs recoverable errors. Errors such as having
   * no web worker are not recoverable will throw an exception instead.
   * @export
   */
  logger: LottieRendererLogger = console;

  /** The onscreen canvas controlled by lottie_worker.js. */
  get onscreenCanvas(): HTMLCanvasElement|null {
    return this.renderRoot.querySelector('#onscreen-canvas');
  }

  private readonly onColorSchemeChanged = () => {
    if (!this.dynamic) return;
    this.refreshAnimationColors();
  };

  /** The worker thread running the lottie library. */
  private worker: Worker|null = null;
  private offscreenCanvas: OffscreenCanvas|null = null;

  /** If the offscreen canvas has been transferred to the Worker thread. */
  private hasTransferredCanvas = false;

  /**
   * Animations are loaded asynchronously, the worker will send an event when
   * it has finished loading.
   */
  private animationIsLoaded = false;

  /**
   * If the canvas is resized before an animation has finished initializing,
   * we wait and send the size once it's loaded fully into the worker.
   */
  private workerNeedsSizeUpdate = false;

  /**
   * If there is a control command (play, pause, stop) before the animation has
   * finished initializing, it should be queued and completed after successful
   * initializaton.
   */
  private workerNeedsControlUpdate = false;
  private playStateInternal = false;

  /**
   * Propagates resize events from the onscreen canvas to the offscreen one.
   */
  private resizeObserver: ResizeObserver|null = null;

  /**
   * Wrapper over a lottie json file that can inject new values for dynamic
   * colors at runtime.
   */
  private processedLottieJSON: ProcessedLottieJSON|null = null;

  /** The reason we sent the last message to the worker. */
  private lastDataMessageSendReason: DataMessageReason = 'unknown';

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: block;
    }

    canvas {
      height: 100%;
      width: 100%;
    }
  `;

  /** @nocollapse */
  static override properties = {
    assetUrl: {type: String, attribute: 'asset-url'},
    autoplay: {type: Boolean, attribute: true},
    loop: {type: Boolean, attribute: true},
    dynamic: {type: Boolean, attribute: true},
  };

  /** @nocollapse */
  static events = {
    ...CrosLottieEvent,
  } as const;

  constructor() {
    super();
    this.assetUrl = '';
    this.autoplay = true;
    this.loop = true;

    // Once all apps are launching with dynamic colors, we can default this to
    // true, or remove this attribute altogether.
    this.dynamic = false;
  }


  override connectedCallback() {
    super.connectedCallback();
    if (!this.worker) {
      this.worker = this.getWorker();
      this.worker.onmessage = (e) => void this.onMessage(e);
    }

    addColorSchemeChangeListener(this.onColorSchemeChanged);
  }

  override firstUpdated() {
    assertExists(
        this.onscreenCanvas,
        'Could not find <canvas> element in lottie-renderer');
    this.resizeObserver =
        new ResizeObserver(this.onCanvasElementResized.bind(this));
    this.resizeObserver.observe(this.onscreenCanvas);
    this.offscreenCanvas = this.onscreenCanvas.transferControlToOffscreen();
    this.lastDataMessageSendReason = 'first-run';
    this.pushAssetURLToWorker();
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    if (this.worker) {
      this.worker.terminate();
      this.worker = null;
    }
    if (this.resizeObserver) {
      this.resizeObserver.disconnect();
    }
    removeColorSchemeChangeListener(this.onColorSchemeChanged);
  }

  override updated(changedProperties: PropertyValues<LottieRenderer>) {
    super.updated(changedProperties);
    const prop = changedProperties.get('assetUrl');
    if (this.assetUrl && prop !== undefined && this.worker) {
      this.assetUrlChanged();
    }
  }

  override render() {
    return html`
        <canvas id='onscreen-canvas'></canvas>
      `;
  }

  /**
   * Play the current animation, and wait for a successful response from the
   * worker thread. This promise will also wait until animation initialization
   * has completed before resolving.
   */
  async play(): Promise<Event> {
    return this.setPlayState(true, CrosLottieEvent.PLAYING);
  }

  /**
   * Pause the current animation, and wait for a successful response from the
   * worker thread. This promise will also wait until animation initialization
   * has completed before resolving.
   */
  async pause(): Promise<Event> {
    return this.setPlayState(false, CrosLottieEvent.PAUSED);
  }

  /**
   * Stop the current animation, and wait for a successful response from the
   * worker thread. Stopping the animation resets it to it's first frame, and
   * pauses it, so there's no need to wait for initialization.
   */
  async stop(): Promise<Event> {
    assertExists(this.worker, 'lottie-renderer has no web worker.');
    this.worker.postMessage({control: {stop: true}});
    return waitForEvent(this, CrosLottieEvent.STOPPED);
  }

  async refreshAnimationColors() {
    if (!this.processedLottieJSON) {
      this.logger.warn('Refresh animation colors failed: No animation data.');
      return;
    }

    await this.updateColorsAndLog();
    this.lastDataMessageSendReason = 'colors-changed';
    this.sendAnimationToWorker(this.processedLottieJSON.getAnimationData());
  }

  private async updateColorsAndLog() {
    if (!this.processedLottieJSON) {
      return;
    }

    // We must await update here to ensure the computed style will be up to date
    // with the new color scheme.
    // TODO: b/274998765 - Investigate if this can be removed when using events.
    this.requestUpdate();
    await this.updateComplete;

    const warnings =
        this.processedLottieJSON.updateColors(getComputedStyle(this));
    for (const warning of warnings) {
      this.logger.warn(
          `Processing lottie json encountered warning: ${warning}`);
    }
  }

  private sendAnimationToWorker(animationData: object) {
    // There are some edge cases where the renderer has been removed from the
    // DOM in between initialization and this function running, which causes
    // the worker to have been terminated. In these cases, we can just early
    // exit without attempting to send anything to the worker.
    if (!this.worker) {
      this.logger.warn('sendAnimationToWorker() invoked with no web worker');
      return;
    }

    const animationConfig: MessageData = ({
      animationData,
      drawSize: this.getCanvasDrawBufferSize(),
      params: {
        loop: this.loop,
        subframeEnabled: false,
        autoplay: this.autoplay,
      },
    });

    // The offscreen canvas can only be transferred across to the WebWorker
    // once, when we first initialize an animation. On subsequent
    // initializations (such as when the asset URL has changed), we avoid
    // re-transferring the canvas and just send across the animation data.
    if (this.hasTransferredCanvas) {
      this.worker.postMessage(animationConfig);
      return;
    }

    this.worker.postMessage(
        {...animationConfig, canvas: this.offscreenCanvas},
        [this.offscreenCanvas as unknown as Transferable]);
    this.hasTransferredCanvas = true;
  }

  /**
   * Load the animation data from `assetUrl` and begin the color token
   * resolution work. If no `assetUrl` was provided, we don't need to try and
   * fetch or send anything to the worker.
   */
  private async pushAssetURLToWorker() {
    if (!this.assetUrl) {
      this.logger.warn('pushAssetURLToWorker() invoked with no asset URL.');
      return;
    }
    let animationData: object|null = null;
    try {
      // In chromium you are not allowed to use `fetch` with `chrome://` URLs,
      // as such we use XMLHttpRequest here.
      animationData = await new Promise((resolve, reject) => {
        const req = new XMLHttpRequest();
        req.responseType = 'json';
        req.open('GET', this.assetUrl, true);
        req.onload = () => {
          resolve(req.response);
        };
        req.onerror = (err) => {
          reject(err);
        };
        req.send(null);
      });
    } catch (error) {
      this.logger.error(`Unable to load JSON from asset url: ${error}`);
    }

    if (!animationData) {
      this.logger.error('Fetched null animation data from asset url.');
      return;
    }

    this.processedLottieJSON = new ProcessedLottieJSON(animationData);
    // For apps using this component without the Jelly flag, they should ignore
    // the dynamic palette and just use the GM2 colors that are inside the
    // JSON file itself.
    if (this.dynamic) {
      await this.updateColorsAndLog();
    }
    if (this.dynamic && this.processedLottieJSON.numMappedColors === 0) {
      this.logger.warn(
          `Found 0 cros.sys.illo tokens in provided asset. Please ` +
          `ensure this animation file has been run through the VisD token ` +
          `resolution script.`);
    }

    this.sendAnimationToWorker(this.processedLottieJSON.getAnimationData());
  }

  private async assetUrlChanged() {
    assertExists(this.worker, 'lottie-renderer has no web worker.');

    // If we have an already loaded animation, stop it from playing and
    // reinitialize.
    if (this.animationIsLoaded) {
      await this.stop();
    }
    this.lastDataMessageSendReason = 'asset-url-changed';
    await this.pushAssetURLToWorker();
  }

  /**
   * Computes the draw buffer size for the canvas. This ensures that the
   * rasterization is crisp and sharp rather than blurry.
   */
  private getCanvasDrawBufferSize() {
    const devicePixelRatio = window.devicePixelRatio;
    const clientRect = this.onscreenCanvas!.getBoundingClientRect();
    const drawSize = {
      width: clientRect.width * devicePixelRatio,
      height: clientRect.height * devicePixelRatio,
    };
    return drawSize;
  }

  /**
   * Handles the canvas element resize event. If the animation isn't fully
   * loaded, the canvas size is sent later, once the loading is done.
   */
  private onCanvasElementResized() {
    if (!this.animationIsLoaded) {
      this.workerNeedsSizeUpdate = true;
      return;
    }
    this.sendCanvasSizeToWorker();
  }

  private sendCanvasSizeToWorker() {
    assertExists(this.worker, 'lottie-renderer has no web worker.');
    this.worker.postMessage({drawSize: this.getCanvasDrawBufferSize()});
  }

  private setPlayState(play: boolean, expectedEvent: CrosLottieEvent) {
    this.playStateInternal = play;
    if (!this.animationIsLoaded) {
      this.workerNeedsControlUpdate = true;
    } else {
      this.sendPlayControlToWorker();
    }
    return waitForEvent(this, expectedEvent);
  }

  private sendPlayControlToWorker() {
    assertExists(this.worker, 'lottie-renderer has no web worker.');
    this.worker.postMessage({control: {play: this.playStateInternal}});
  }

  private onMessage(event: MessageEvent) {
    const message = event.data.name;
    switch (message) {
      case 'initialized':
        this.animationIsLoaded = true;
        this.fire(CrosLottieEvent.INITIALIZED, this.lastDataMessageSendReason);
        this.sendPendingInfo();
        break;
      case 'playing':
        this.fire(CrosLottieEvent.PLAYING);
        break;
      case 'paused':
        this.fire(CrosLottieEvent.PAUSED);
        break;
      case 'stopped':
        this.fire(CrosLottieEvent.STOPPED);
        break;
      case 'resized':
        this.fire(CrosLottieEvent.RESIZED, event.data.size);
        break;
      default:
        this.logger.warn(`unknown message type received: ${message}`);
        break;
    }
  }

  private fire(
      event: CrosLottieEvent,
      detail: ResizeDetail|DataMessageReason|null = null) {
    this.dispatchEvent(
        new CustomEvent(event, {bubbles: true, composed: true, detail}));
  }

  /**
   * Called once the animation is fully loaded into the worker. Sends any
   * size or control information that may have arrived while the animation
   * was not yet fully loaded.
   */
  private sendPendingInfo() {
    if (this.workerNeedsSizeUpdate) {
      this.workerNeedsSizeUpdate = false;
      this.sendCanvasSizeToWorker();
    }
    if (this.workerNeedsControlUpdate) {
      this.workerNeedsControlUpdate = false;
      this.sendPlayControlToWorker();
    }
  }
}

customElements.define('cros-lottie-renderer', LottieRenderer);

declare global {
  interface HTMLElementEventMap {
    [LottieRenderer.events.INITIALIZED]: CrosLottieInitializedEvent;
    [LottieRenderer.events.PAUSED]: Event;
    [LottieRenderer.events.PLAYING]: Event;
    [LottieRenderer.events.RESIZED]: Event;
    [LottieRenderer.events.STOPPED]: Event;
  }

  interface HTMLElementTagNameMap {
    'cros-lottie-renderer': LottieRenderer;
  }
}
