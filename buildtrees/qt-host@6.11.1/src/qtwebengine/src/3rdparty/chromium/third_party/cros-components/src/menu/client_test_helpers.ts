/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @fileoverview A set of helpers clients can use to interact with a cros-menu
 * in tests.
 */

import {LitElement} from 'lit';

import {waitForComponentReady} from '../client_test_helpers';
import {assert, assertExists, castExists} from '../helpers/helpers';

import {Menu} from './menu';
import {MenuItem} from './menu_item';
import {SubMenuItem} from './sub_menu_item';

/**
 * Type for a function that can be used as a filter for a set of menuItem and
 * SubMenuItems.
 */
export type MenuItemFilter = (menuItem: MenuItem|SubMenuItem) => boolean;

/** Options for opening a menu. */
export interface OpenMenuOptions {
  /**
   * What input method should be used to open the menu. Defaults to programtic.
   */
  method: 'programmatic'|'arrow-down';
}

/** Waits for `eventName` to be fired on `menu`. */
export async function waitForMenuEvent(
    menu: Menu, eventName: 'opened'|'closed') {
  await new Promise(
      r => void menu.addEventListener(eventName, r, {once: true}));
}

function extractMenuFromSubMenuItem(subMenuItem: SubMenuItem) {
  const castFailureMsg = `cros-submenu-item[headline="${
      subMenuItem.headline}"] has no cros-menu inside it`;
  return castExists(subMenuItem.querySelector('cros-menu'), castFailureMsg);
}

/**
 * Finds a menu item or a submenu item that passes the provided
 * `menuItemFilter`. Performs a DFS through the menu and its subMenus to find
 * the element and returns the menuItem that passes the filter and the path of
 * nodes that were traversed through to get to it.
 */
function findMenuItem(
    menu: Menu,
    menuItemFilter: MenuItemFilter,
    ): {target: MenuItem|SubMenuItem|null, path: Array<Menu|SubMenuItem>} {
  for (const child of menu.children) {
    if (!isMenuItem(child) && !isSubmenuItem(child)) {
      continue;
    }
    if (menuItemFilter(child)) {
      // We've found the item.
      return {target: child, path: [menu]};
    }
    if (isSubmenuItem(child)) {
      // We have not found the item but we have a submenu we can search through.
      const submenu = extractMenuFromSubMenuItem(child);
      const subsearch = findMenuItem(submenu, menuItemFilter);
      if (subsearch.target) {
        return {
          target: subsearch.target,
          path: [menu, child].concat(...subsearch.path),
        };
      }
    }
  }

  return {target: null, path: [menu]};
}

function isMenuItem(element: Element): element is MenuItem {
  return element.tagName === 'CROS-MENU-ITEM';
}

function isSubmenuItem(element: Element): element is SubMenuItem {
  return element.tagName === 'CROS-SUB-MENU-ITEM';
}

async function openSubMenuItem(subMenuItem: SubMenuItem) {
  assert(
      subMenuItem.isConnected,
      'openSubMenuItem provided with subMenuItem which is disconnected from the DOM.');
  const submenu = extractMenuFromSubMenuItem(subMenuItem);
  subMenuItem.show();
  // In case the menu has an animation we wait for the `opened` event which
  // fires once the animation is done.
  await waitForMenuEvent(submenu, 'opened');
}

/** Opens a menu and waits for it to finish animating if needed. */
export async function openMenu(menu: Menu, options: OpenMenuOptions = {
  method: 'programmatic'
}) {
  assert(
      menu.isConnected,
      'closeMenu provided with menu which is disconnected from the DOM.');
  if (options.method === 'programmatic') {
    menu.show();
  } else {
    assertExists(
        menu.anchorElement,
        'No anchor element for menu, can not open via ArrowDown');
    menu.anchorElement.dispatchEvent(
        new KeyboardEvent('keydown', {key: 'ArrowDown'}));
  }

  // In case the menu has an animation we wait for the `opened` event which
  // fires once the animation is done.
  await waitForMenuEvent(menu, 'opened');
}

/**
 * A simple menuItemFilter which matches the first child of a menu's first
 * submenu.
 */
export function isSubMenuMenuItem(menuItem: MenuItem|SubMenuItem) {
  return menuItem.parentElement?.slot === 'submenu';
}

/** Closes a menu and waits for it to finish animating if needed. */
export async function closeMenu(menu: Menu) {
  assert(
      menu.isConnected,
      'closeMenu provided with menu which is disconnected from the DOM.');

  menu.close();
  // In case the menu has an animation we wait for the `closed` event which
  // fires once the animation is done.
  await waitForMenuEvent(menu, 'closed');
}

/**
 * Finds a menu item or a submenu item that passes `menuItemFilter` and opens
 * all needed menus and submenus to ensure the item is visible. Returns a
 * reference to the menuItem DOM element that was revealed.
 */
export async function revealMenuItem(
    menu: Menu, menuItemFilter: MenuItemFilter): Promise<MenuItem|SubMenuItem> {
  await menu.updateComplete;
  // Flush the microtask queue to ensure all menu children have finished
  // rendering.
  await waitForComponentReady(menu);

  const search = findMenuItem(menu, menuItemFilter);
  assertExists(
      search.target, 'No menu item found that passes given menuItemFilter');
  assert(
      search.path.length > 0,
      'Test Code Error: findMenuItem returned target but no path.');

  const base = search.path[0] as Menu;
  const subMenuItems = search.path.slice(1).filter(isSubmenuItem);
  await openMenu(base);
  for (const subMenuItem of subMenuItems) {
    await openSubMenuItem(subMenuItem);
  }
  return search.target;
}

/**
 * Finds a menu item or a submenu item and clicks on it. Will ensure all parent
 * menus for the item are opened before clicking.
 */
export async function triggerMenuItem(
    menu: Menu, menuItemFilter: MenuItemFilter) {
  const target = await revealMenuItem(menu, menuItemFilter);
  await clickMenuItem(target);
  await menu.updateComplete;
}

/**
 * Clicks a provided menu item ignoring if it's currently in an open menu or
 * not.
 */
export async function clickMenuItem(menuItem: MenuItem|SubMenuItem) {
  // The interactive element tVhat click and focus events are fired on is the
  // list-item inside of the md-menu-item.
  const mdListItem = castExists<LitElement|null>(
      menuItem.renderRoot.querySelector('md-menu-item'),
      `cros-menu-item[headline=${
          menuItem.headline}] has no md-menu-item inside of it.`);
  const interactiveElement = castExists<HTMLElement|null>(
      mdListItem.renderRoot.querySelector('.list-item'),
      `cros-menu-item[headline=${
          menuItem
              .headline}] md-list-item has no .list-item element inside of it.`);
  interactiveElement.click();
}
