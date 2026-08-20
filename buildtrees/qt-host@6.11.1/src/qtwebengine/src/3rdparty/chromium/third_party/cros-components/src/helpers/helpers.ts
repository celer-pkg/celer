/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @fileoverview A set of chromium safe helper functions. Many of these exist
 * in google3 already but we reimplement so they can also be imported in
 * chromium.
 */

/**
 * Asserts that `arg` is not null or undefined and informs typescript of this so
 * that code that runs after this check will know `arg` is not null. If `arg` is
 * null, throws an error with `msg`.
 *
 * NOTE: unlike the internal counterpart this ALWAYS throws an error when given
 * null or undefined. This is not compiled out for prod.
 */
export function assertExists<A>(
    arg: A, msg: string): asserts arg is NonNullable<A> {
  if (!arg) {
    throw new Error(`Assertion Error: ${msg}`);
  }
}

/**
 * Asserts that `condition` is true and throws an error otherwise.
 *
 * NOTE: unlike the internal counterpart this ALWAYS throws an error when given
 * null or undefined. This is not compiled out for prod.
 */
export function assert(condition: boolean, msg: string) {
  if (!condition) {
    throw new Error(`Assertion Error: ${msg}`);
  }
}

/**
 * Asserts that `arg` is not null and returns it. If `arg` is null, throws an
 * error with `msg`.
 *
 * NOTE: unlike the internal counterpart this ALWAYS throws an error when given
 * null or undefined. This is not compiled out for prod.
 */
export function castExists<A>(arg: A, msg: string|null = null): NonNullable<A> {
  if (arg === null || arg === undefined) {
    throw new Error(
        msg ? `Assertion Error: ${msg}` : `The reference is ${arg}`);
  }
  return arg;
}

/** Converts a given hex string to a [r, g, b] array. */
export function hexToRgb(hexString: string): [number, number, number] {
  const rHex = hexString.substring(1, 3);
  const gHex = hexString.substring(3, 5);
  const bHex = hexString.substring(5, 7);

  return [
    Number(`0x${rHex}`),
    Number(`0x${gHex}`),
    Number(`0x${bHex}`),
  ];
}

/**
 * Whether `event.target` should forward a click event to its children and local
 * state. False means a child should already be aware of the click.
 */
export function shouldProcessClick(event: MouseEvent) {
  // Event must start at the event target.
  if (event.currentTarget !== event.target) {
    return false;
  }
  // Event must not be retargeted from shadowRoot.
  if (event.composedPath()[0] !== event.target) {
    return false;
  }
  // Target must not be disabled; this should only occur for a synthetically
  // dispatched click.
  if ((event.target as EventTarget & {disabled: boolean}).disabled) {
    return false;
  }
  return true;
}

/**
 * Returns the chain of active elements. This is the actual activeElement,
 * preceeded by all the containers with shadowroots.
 */
export function shadowPiercingActiveElements(): Element[] {
  let activeElement = document.activeElement;
  const activeElements = activeElement ? [activeElement] : [];
  while (activeElement && activeElement.shadowRoot) {
    activeElement = activeElement.shadowRoot.activeElement;
    if (activeElement) activeElements.push(activeElement);
  }
  return activeElements;
}

/**
 * Returns the innermost activeElement with type `tag`, or null if there is no
 * activeElement of this type.
 */
export function shadowPiercingActiveElement(tag: keyof HTMLElementTagNameMap):
    HTMLElementTagNameMap[typeof tag]|null {
  const upperTag = tag.toUpperCase();
  const chain = shadowPiercingActiveElements();
  for (const el of chain.reverse()) {
    if (el.tagName === upperTag) return el as HTMLElementTagNameMap[typeof tag];
  }
  return null;
}

/**
 * True if the page is read right-to-left. Do not assume this calculation is
 * cheap.
 */
export function isRTL(el: HTMLElement): boolean {
  // Search for the closest ancestor with a direction.
  let dirEl: typeof el|null = el;
  while (dirEl && !dirEl.dir) {
    dirEl = dirEl.parentElement;
  }

  if (dirEl && dirEl.dir) {
    return dirEl.dir === 'rtl';
  } else {
    return document.documentElement.dir === 'rtl';
  }
}
