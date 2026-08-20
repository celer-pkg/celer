/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {hexToRgb} from '../helpers/helpers';

/**
 * The list of Sparkle illustration tokens. See the documentation comment for
 * CROS_TOKENS below for more details.
 */
export const CROS_TOKENS_SPARKLE_ILLUSTRATION = [
  'cros.sys.illo.analog',
  'cros.sys.illo.complement',
  'cros.sys.illo.muted',
  'cros.sys.illo.on_gradient',
] as const;

/**
 * The list of Sparkle system tokens. See the documentation comment for
 * CROS_TOKENS below for more details.
 */
export const CROS_TOKENS_SPARKLE_SYSTEM = [
  'cros.sys.analog',
  'cros.sys.analog-variant',
  'cros.sys.muted',
  'cros.sys.muted-variant',
  'cros.sys.complement',
  'cros.sys.complement-variant',
] as const;

/**
 * The list of tokens that are used to identify shapes and colors in Lottie
 * animation data. If token names change, we will need to update this set.
 * Existing token names are very unlikely to change, it's more likely that new
 * tokens may be added if illustration palettes become more complex. There
 * is currently no easy way to generate this list in Google3, and as a small
 * set it can be manually maintained, but in future a more robust solution may
 * be needed. See go/cros-tokens (internal).
 */
export const CROS_TOKENS = [
  'cros.sys.illo.color1',
  'cros.sys.illo.color1.1',
  'cros.sys.illo.color1.2',
  'cros.sys.illo.color2',
  'cros.sys.illo.color3',
  'cros.sys.illo.color4',
  'cros.sys.illo.color5',
  'cros.sys.illo.color6',
  'cros.sys.illo.base',
  'cros.sys.illo.secondary',
  'cros.sys.illo.on_primary_container',

  /**
   * Colors for gradient animatons.
   */
  ...CROS_TOKENS_SPARKLE_SYSTEM,

  /**
   * Colors for gradient illustrations.
   */
  ...CROS_TOKENS_SPARKLE_ILLUSTRATION,

  /**
   * These are colors outside of the standard illo palette. Some animations
   * need to modify the base background color depending on the surface so
   * contain tokens for app base, card color etc,
   */
  'cros.sys.app_base',
  'cros.sys.app_base_shaded',
  'cros.sys.base_elevated',
  'cros.sys.illo.card.color5',
  'cros.sys.illo.card.on_color5',
  'cros.sys.illo.card.color4',
  'cros.sys.illo.card.on_color4',
  'cros.sys.illo.card.color3',
  'cros.sys.illo.card.on_color3',
  'cros.sys.illo.card.color2',
  'cros.sys.illo.card.on_color2',
  'cros.sys.illo.card.color1',
  'cros.sys.illo.card.on_color1',
] as const;

/** Type of the values in the CROS_TOKENS array. */
export type CrosToken = typeof CROS_TOKENS[number];


/** String variant of the name field used for comparison during parsing. */
const LOTTIE_GRADIENT_FILL_TYPE = 'gf';

/** O(1) way to look up if a token is valid. */
const crosTokenLookup = new Set<string>(CROS_TOKENS);

/**
 * A structure in the JSON data that should correspond to something that needs
 * to be dynamically colored.
 */
declare interface DynamicallyColoredObject {
  /**
   * The name of a shape. For Material3 compliant animations, this will be the
   * token name of the color to apply, and we use this field to detect which
   * parts of the JSON data correspond to colored structures.
   */
  nm: string;
  /**
   * Optional type of the structure. This field will be set to the value of
   * `LOTTIE_GRADIENT_FILL_TYPE` if it is a gradient fill structure. If not, we
   * assume that it is a regular shape (like a stroke or fill).
   */
  ty?: string;
}

/**
 * This definition of this type is taken from
 * https://lottiefiles.github.io/lottie-docs/concepts/#colors
 */
export declare interface LottieShape extends DynamicallyColoredObject {
  /** If the color is represented as an array of four floats: [r, g, b, a]. */
  c?: {k: LottieRGBAArray};
  /** If the color is represented as a hex string (less common than `c`). */
  sc?: string;
}


/**
 * This definition of this type is taken from
 * https://lottiefiles.github.io/lottie-docs/shapes/#gradients
 */
export declare interface LottieGradientShape extends DynamicallyColoredObject {
  /** Gradient color type. */
  g: {
    /** The number of colors in this gradient. */
    p: number,
    /** Animated Gradient Colors. */
    k: {
      /**
       * Whether the property is animated, should be 0 for this fill type which
       * means the following `k` type is an array of values rather than
       * keyframes.
       */
      a: 0,
      /**
       * Gradient values over time.
       * Values are grouped in contiguous sub-arrays like [t, r, g, b], where
       * [r,g,b] is a decimal representation of color channel values at time
       * `t`. There should be `p` number of sub arrays. If alpha values are
       * included, they are at the end of the array, grouped as [t, a], where a
       * is the alpha value to apply at time `t`. If alpha values are not
       * included, the k.length will be p*4, with alpha values it is p*6. The
       * relationship between the location of a set of [t, r, g, b] values and
       * an alpha value, is described by a_n = 4p + 2i + 1.
       */

      k: number[]
    }
  };
}


function isCrosToken(token: string): token is CrosToken {
  return crosTokenLookup.has(token);
}

/**
 * Takes in a css color in one of the following formats
 *   - #rrggbb
 *   - #rrggbbaa
 *   - rgb(0-255,0-255,0-255)
 *   - rgba(0-255,0-255,0-255,0-1)
 *   - color(srgb 0-1 0-1 0-1)
 *   - color-mix() see http://go/mdn/CSS/color_value/color-mix
 * And returns the color in the #rrggbbaa format.
 */
export function toRGBAHexString(color: string) {
  /**
   * Converts a string with a number between 0 and 255 in it to a 2 character
   * hex string.
   */
  function fourBitNumberStringToHex(value: string) {
    return Number(value.trim()).toString(16).padStart(2, '0');
  }

  /**
   * Converts a string with a number between 0 and 1 in it to a 2 character
   * hex string.
   */
  function normalizedNumberStringToHex(value: string) {
    const n = Math.floor(Number(value.trim()) * 255);
    return n.toString(16).padStart(2, '0');
  }

  color = color.trim();
  if (color.startsWith('color-mix(')) {
    // color-mix's need to be actually be put into the DOM and rendered for us
    // to get a final rgb color. We do that here however getComputedStyle can
    // return any number of rgb formats so we just override color and pass
    // through to the rest of the logic in this function to noramlize to
    // #rrggbbaa.
    const tempDiv = document.createElement('div');
    tempDiv.style.backgroundColor = color;
    tempDiv.style.display = 'none';
    document.body.appendChild(tempDiv);
    color = getComputedStyle(tempDiv).backgroundColor.trim();
    tempDiv.remove();
  }

  // This is not a exhaustive list of all possible ways to represent a color
  // in CSS. Instead this covers all forms the semantic variable generator
  // or getComputedStyle() is known to output.
  if (color.startsWith('#') && color.length === 7) {
    // #rrggbb.
    return `${color}ff`;
  } else if (color.startsWith('#') && color.length === 9) {
    // #rrggbbaa.
    return color;
  } else if (color.startsWith('rgb(')) {
    // rgb(r,g,b).
    const [r, g, b] = color.substring(4, color.length - 1)
                          .split(',')
                          .map(fourBitNumberStringToHex);

    return `#${r}${g}${b}ff`;
  } else if (color.startsWith('rgba(')) {
    // rgba(r,g,b,a).
    const parts = color.substring(5, color.length - 1).split(',');
    const [r, g, b] = parts.slice(0, 3).map(fourBitNumberStringToHex);
    const a = normalizedNumberStringToHex(parts[3] as string);

    return `#${r}${g}${b}${a}`;
  } else if (color.startsWith('color(srgb')) {
    // color(srgb r g b).
    color = color.replace(/\s+/g, ' ');
    const [r, g, b] = color.substring(11, color.length - 1)
                          .split(' ')
                          .map(normalizedNumberStringToHex);

    return `#${r}${g}${b}ff`;
  }

  throw new Error(`Could not parse color: "${color}"`);
}

/**
 * A type for storing a css variable and a list of known shapes within the
 * currently loaded animation that have the css variable color applied.
 * There will be one of these structures per token in CROS_TOKENS that
 * appears in the animation data.
 */
interface TokenColor {
  cssVar: string;
  shapes: LottieShape[];
  gradients: Array<{location: LottieGradientShape, stopIndex: number}>;
}

/**
 * A fixed length array type for encoding Lottie colors. The format is [r, g, b,
 * alpha], where each entry is a value between [0-1].
 */
export type LottieRGBAArray = [number, number, number, number];

/**
 * Helper function for converting from a lottie rgba string to a hex string.
 */
export function convertLottieRGBAToHex(rgba: LottieRGBAArray) {
  return '#' +
      rgba.map(n => n * 255)
          .map(n => Math.round(n))
          .map(n => n.toString(16))
          .map(n => n.padStart(2, '0'))
          .join('');
}

/**
 * Helper function for converting between the hexadecimal string we get from the
 * computed style to LottieRGBAArray type. Since these come directly
 * from the computed style and color pipeline, we can be confident that we are
 * only going to be parsing 8 digit hexadecimal strings.
 */
export function convertHexToLottieRGBA(hexString: string): LottieRGBAArray {
  let r;
  let g;
  let b;
  let alpha;
  if (hexString.length === 9) {
    // Assume #rrggbbaa format.
    const hexRgb = hexString.slice(0, -2);
    const alphaString = hexString.slice(-2);
    [r, g, b] = hexToRgb(hexRgb);
    alpha = Number(`0x${alphaString}`);
  } else {
    // Assume #rrggbb format.
    [r, g, b] = hexToRgb(hexString);
    alpha = 255;
  }

  return [r / 255, g / 255, b / 255, alpha / 255];
}

/**
 * Takes in a cros token and returns the corresponding css variable.
 * Eg: cros.sys.illo.base-abc -> --cros-sys-illo-base_abc
 */
export function convertTokenToCssVariable(token: string) {
  const parts = token.split('.');
  const namespaces = parts.slice(0, parts.length - 1);
  // Chromium assumes all array accesses are of type|undefined, cast to a string
  // since we know for a fact that token.split('.') always returns at least 1
  // item.
  const name = (parts[parts.length - 1] as string).replaceAll('-', '_');

  if (namespaces.length === 0) {
    return `--${name}`;
  }
  return `--${namespaces.join('-')}-${name}`;
}

/** Does a walk over a lottie json object and yields each node in DFS order. */
export function*
    walkLottieJson(lottieJson: object|null): IterableIterator<object> {
  if (lottieJson === null) {
    return;
  }

  if (typeof lottieJson === 'object') {
    for (const value of Object.values(lottieJson)) {
      yield* walkLottieJson(value);
    }
  }
  yield lottieJson;
}

function getOrCreateTokenColor(
    colors: Map<string, TokenColor>, tokenName: CrosToken) {
  if (!colors.has(tokenName)) {
    colors.set(tokenName, {
      cssVar: convertTokenToCssVariable(tokenName),
      shapes: [],
      gradients: []
    });
  }
  return (colors.get(tokenName) as TokenColor);
}

function isValidShapeName(tokenName: string|null): tokenName is CrosToken {
  if (tokenName === null) {
    return false;
  }

  const tokens = tokenName.split(',');
  return tokens.some(isCrosToken);
}

/**
 * A class wrapping a lottie JSON file that has been injected with dynamic
 * variables that can be hot swapped with new values at runtime.
 */
export class ProcessedLottieJSON {
  private colorReferences = new Map<CrosToken, TokenColor>();
  numMappedColors = 0;

  constructor(private readonly animationData: object) {
    this.traverse(this.animationData);
  }

  /**
   * Traverses through a jsonObject, looking for known keys and tokens, and
   * saving them in the `shapes` and `gradients` map.
   */
  private traverse(jsonObj: object) {
    for (const node of walkLottieJson(jsonObj)) {
      const shape =
          (node as Partial<DynamicallyColoredObject>|
           Partial<LottieGradientShape>);
      const shapeName = shape.nm || null;

      if (!isValidShapeName(shapeName)) {
        continue;
      }

      this.numMappedColors++;

      // Attempt to parse the object as a gradient, otherwise we assume it is
      // a regular shape. If more complex animation types get added, this
      // logic will need to be updated along with the types.
      if (shape.ty === LOTTIE_GRADIENT_FILL_TYPE) {
        const stops = shapeName.split(',');
        for (let i = 0; i < stops.length; i++) {
          const stopTokenName = stops[i] as CrosToken;
          if (!isCrosToken(stopTokenName)) {
            continue;
          }

          const tokenColor =
              getOrCreateTokenColor(this.colorReferences, stopTokenName);
          tokenColor.gradients.push(
              {location: shape as LottieGradientShape, stopIndex: i});
        }
      } else {
        const tokenColor =
            getOrCreateTokenColor(this.colorReferences, shapeName);
        tokenColor.shapes.push(shape as LottieShape);
      }
    }
  }

  /**
   * Finds all dynamic colors in the underlying json file and updates their
   * values to match the current color palette in the document.
   *
   * @param styles A CSSStyleDecleration from getComputedStyles() on the element
   * to pull css variable values from.
   * @returns A list of warnings in attempting to update the color on the json
   * file.
   */
  updateColors(styles: CSSStyleDeclaration): string[] {
    const warnings = [];
    for (const color of this.colorReferences.values()) {
      let computedColor;
      try {
        computedColor =
            toRGBAHexString(styles.getPropertyValue(color.cssVar).trim());
      } catch {
        warnings.push(`Unable to get value of ${
            color.cssVar}. Are token css vars installed in this page?`);
        computedColor = '#000000';
      }
      const colorArray = convertHexToLottieRGBA(computedColor);
      for (const location of color.shapes) {
        if (location.c) {
          location.c.k = colorArray;
        } else if (location.sc) {
          location.sc = computedColor;
        } else {
          warnings.push(
              `Unable to assign color to shape: ${JSON.stringify(location)}`);
        }
      }

      for (const {location, stopIndex} of color.gradients) {
        const gradientFillPoints = location.g.k.k;
        gradientFillPoints[(4 * stopIndex) + 1] = colorArray[0];
        gradientFillPoints[(4 * stopIndex) + 2] = colorArray[1];
        gradientFillPoints[(4 * stopIndex) + 3] = colorArray[2];
      }
    }

    return warnings;
  }

  getAnimationData() {
    return this.animationData;
  }
}
