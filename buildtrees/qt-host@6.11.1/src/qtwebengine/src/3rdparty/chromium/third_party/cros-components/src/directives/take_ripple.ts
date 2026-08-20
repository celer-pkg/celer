/**
 * @fileoverview A lit directive which intercept various events with the purpose
 * of suppressing the ripple in parent elements.
 *
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {Directive, directive, ElementPart, PartInfo, PartType} from 'lit/directive';

class TakeRipple extends Directive {
  constructor(partInfo: PartInfo) {
    super(partInfo);
    if (partInfo.type !== PartType.ELEMENT) {
      throw new Error(
          'The `TakeRipple` directive must be used in the element,' +
          ' i.e. <p ${takeRipple}/>');
    }
    const el = (partInfo as ElementPart).element;
    el.addEventListener('click', this.take);
    el.addEventListener('pointerdown', this.take);
    el.addEventListener('pointerup', this.take);
    el.addEventListener('pointerenter', this.take);
    el.addEventListener('pointerleave', this.take);
  }

  private take(e: Event) {
    e.stopPropagation();
    e.preventDefault();
  }

  override render() {
    return '';
  }
}

/**
 * Prevent a ripple from playing on parent elements. This may suppress other
 * pointer styles as a side effect.
 */
export const takeRipple = directive(TakeRipple);
