# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

from selenium.webdriver import Chrome
from selenium.webdriver.chrome.service import Service as ChromeService
from selenium.webdriver.chrome.options import Options as ChromeOptions
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.actions.action_builder import ActionBuilder
from selenium.webdriver.common.actions.pointer_actions import PointerActions
from selenium.webdriver.common.actions.interaction import POINTER_TOUCH
from selenium.webdriver.common.actions.pointer_input import PointerInput
from selenium.webdriver.common.by import By
from selenium.webdriver.support.expected_conditions import presence_of_element_located
from selenium.webdriver.support.ui import WebDriverWait

import os
import time
import unittest
from enum import Enum, auto

class WidgetTestCase(unittest.TestCase):
    def setUp(self):
        chromedriver_path = os.getenv('WASM_CHROMEDRIVER_PATH_JSPI')
        chromedriver_args = ""

        if chromedriver_path:
            chromedriver_args = os.getenv('WASM_CHROMEDRIVER_PATH_JSPI_ARGS')
        else:
            chromedriver_path = os.getenv('CHROMEDRIVER_PATH')

        if chromedriver_path:
            options = ChromeOptions()
            args = chromedriver_args.split()
            for arg in args:
                options.add_argument(arg)
            self._driver = Chrome(service=ChromeService(executable_path=chromedriver_path), options=options)
        else:
            self._driver = Chrome()
        self._driver.maximize_window()
        self._driver.get(
            'http://localhost:8001/tst_qwasmwindow_harness_run.html')
        self._test_sandbox_element = WebDriverWait(self._driver, 30).until(
            presence_of_element_located((By.ID, 'test-sandbox'))
        )
        self.addTypeEqualityFunc(Color, assert_colors_equal)
        self.addTypeEqualityFunc(Rect, assert_rects_equal)

    def test_buttons(self):
        screen = Screen(self._driver, ScreenPosition.FIXED,
                    x=0, y=0, width=1200, height=1200)

        self._driver.execute_script(
            f'''
                instance.run_QML('buttons.qml');
            '''
        )
        # Check initial status
        checkboxa1 = screen.find_element(By.ID, f'checkboxa1')
        checkboxa2 = screen.find_element(By.ID, f'checkboxa2')
        radiobuttonb1 = screen.find_element(By.ID, f'radiobuttonb1')
        radiobuttonb2 = screen.find_element(By.ID, f'radiobuttonb2')
        pushbuttonc1 = screen.find_element(By.ID, f'pushbuttonc1')
        pushbuttonc2 = screen.find_element(By.ID, f'pushbuttonc2')
        checkboxa1DB = screen.find_element(By.ID, checkboxa1.get_attribute("aria-describedby"));
        checkboxa2DB = screen.find_element(By.ID, checkboxa2.get_attribute("aria-describedby"));
        radiobuttonb1DB = screen.find_element(By.ID, radiobuttonb1.get_attribute("aria-describedby"));
        radiobuttonb2DB = screen.find_element(By.ID, radiobuttonb2.get_attribute("aria-describedby"));
        pushbuttonc1DB = screen.find_element(By.ID, pushbuttonc1.get_attribute("aria-describedby"));
        pushbuttonc2DB = screen.find_element(By.ID, pushbuttonc2.get_attribute("aria-describedby"));

        self.assertEqual(checkboxa1DB.get_attribute("innerHTML"), "Button A1");
        self.assertEqual(checkboxa2DB.get_attribute("innerHTML"), "Button A2");
        self.assertEqual(radiobuttonb1DB.get_attribute("innerHTML"), "Button B1");
        self.assertEqual(radiobuttonb2DB.get_attribute("innerHTML"), "Button B2");
        self.assertEqual(pushbuttonc1DB.get_attribute("innerHTML"), "Button C1");
        self.assertEqual(pushbuttonc2DB.get_attribute("innerHTML"), "Button C2");

        self.assertEqual(checkboxa1.get_attribute("checked"), None);
        self.assertEqual(checkboxa2.get_attribute("checked"), None);
        self.assertEqual(checkboxa1.get_attribute("type"), "checkbox");
        self.assertEqual(checkboxa2.get_attribute("type"), "checkbox");
        self.assertEqual(checkboxa1.get_attribute("aria-label"), "ButtonA1");
        self.assertEqual(checkboxa2.get_attribute("aria-label"), "ButtonA2");


        self.assertEqual(radiobuttonb1.get_attribute("checked"), None);
        self.assertEqual(radiobuttonb2.get_attribute("checked"), None);
        self.assertEqual(radiobuttonb1.get_attribute("type"), "radio");
        self.assertEqual(radiobuttonb2.get_attribute("type"), "radio");
        self.assertEqual(radiobuttonb1.get_attribute("aria-label"), "ButtonB1");
        self.assertEqual(radiobuttonb2.get_attribute("aria-label"), "ButtonB2");

        self.assertEqual(pushbuttonc1.get_attribute("checked"), None);
        self.assertEqual(pushbuttonc2.get_attribute("checked"), None);
        self.assertEqual(pushbuttonc1.get_attribute("type"), "submit");
        self.assertEqual(pushbuttonc2.get_attribute("type"), "submit");
        self.assertEqual(pushbuttonc1.get_attribute("aria-label"), "ButtonC1");
        self.assertEqual(pushbuttonc2.get_attribute("aria-label"), "ButtonC2");

        #time.sleep(3600)
        # Check focus
        screen.focus(checkboxa1);
        self.assertTrue(screen.has_focus(checkboxa1));
        checkboxa1.send_keys("\t");
        self.assertTrue(screen.has_focus(checkboxa2));
        checkboxa2.send_keys("\t");
        self.assertTrue(screen.has_focus(radiobuttonb1));
        radiobuttonb1.send_keys("\t");
        self.assertTrue(screen.has_focus(radiobuttonb2));
        radiobuttonb2.send_keys("\t");
        self.assertTrue(screen.has_focus(pushbuttonc1));
        pushbuttonc1.send_keys("\t");
        self.assertTrue(screen.has_focus(pushbuttonc2));

        screen.focus(checkboxa1);
        # Check behavior checkboxes
        screen.click(checkboxa1);
        self.assertEqual(checkboxa1.get_dom_attribute("checked"), "true");
        self.assertEqual(checkboxa2.get_dom_attribute("checked"), None);

        screen.click(checkboxa2);
        self.assertEqual(checkboxa1.get_dom_attribute("checked"), "true");
        self.assertEqual(checkboxa2.get_dom_attribute("checked"), "true");

        screen.click(checkboxa1);
        self.assertEqual(checkboxa1.get_dom_attribute("checked"), None);
        self.assertEqual(checkboxa2.get_dom_attribute("checked"), "true");

        screen.click(checkboxa2);
        self.assertEqual(checkboxa1.get_dom_attribute("checked"), None);
        self.assertEqual(checkboxa2.get_dom_attribute("checked"), None);

        # Check behavior radiobuttons
        screen.click(radiobuttonb1);
        self.assertEqual(radiobuttonb1.get_dom_attribute("checked"), "true");
        self.assertEqual(radiobuttonb2.get_dom_attribute("checked"), None);

        screen.click(radiobuttonb2);
        self.assertEqual(radiobuttonb1.get_dom_attribute("checked"), None);
        self.assertEqual(radiobuttonb2.get_dom_attribute("checked"), "true");

        screen.click(radiobuttonb1);
        self.assertEqual(radiobuttonb1.get_dom_attribute("checked"), "true");
        self.assertEqual(radiobuttonb2.get_dom_attribute("checked"), None);

        screen.click(radiobuttonb2);
        self.assertEqual(radiobuttonb1.get_dom_attribute("checked"), None);
        self.assertEqual(radiobuttonb2.get_dom_attribute("checked"), "true");

        # Check behaviour push buttons
        screen.click(pushbuttonc1);
        screen.click(pushbuttonc2);
        self.assertEqual(pushbuttonc1.get_attribute("aria-label"), "ButtonC1 - clicked");
        self.assertEqual(pushbuttonc2.get_attribute("aria-label"), "ButtonC2 - clicked");

        self._driver.execute_script(
            f'''
                instance.kill_QML();
            '''
        )

    def test_text(self):
        screen = Screen(self._driver, ScreenPosition.FIXED,
                    x=0, y=0, width=1200, height=1200)

        self._driver.execute_script(
            f'''
                instance.run_QML('text.qml');
            '''
        )

        text1 = screen.find_element(By.ID, f'text1');
        text2 = screen.find_element(By.ID, f'text2');
        text3 = screen.find_element(By.ID, f'text3');
        text4 = screen.find_element(By.ID, f'text4');
        texts1 = screen.find_element(By.ID, f'texts1');
        texts2 = screen.find_element(By.ID, f'texts2');
        texts3 = screen.find_element(By.ID, f'texts3');
        texts4 = screen.find_element(By.ID, f'texts4');

        texth1 = screen.find_element(By.ID, f'texth1');
        texth4 = screen.find_element(By.ID, f'texth4');

        textne1 = screen.find_element(By.ID, f'textne1');
        textne4 = screen.find_element(By.ID, f'textne4');

        textpe1 = screen.find_element(By.ID, f'textpe1');
        textpe4 = screen.find_element(By.ID, f'textpe4');

        text1DB = screen.find_element(By.ID, text1.get_attribute("aria-describedby"));
        text2DB = screen.find_element(By.ID, text2.get_attribute("aria-describedby"));
        text3DB = screen.find_element(By.ID, text3.get_attribute("aria-describedby"));
        #text4DB = screen.find_element(By.ID, text4.get_attribute("aria-describedby"));
        texts1DB = screen.find_element(By.ID, texts1.get_attribute("aria-describedby"));
        texts2DB = screen.find_element(By.ID, texts2.get_attribute("aria-describedby"));
        texts3DB = screen.find_element(By.ID, texts3.get_attribute("aria-describedby"));
        #texts4DB = screen.find_element(By.ID, texts4.get_attribute("aria-describedby"));

        self.assertEqual(text1.get_attribute("value"), "text is TextInput");
        self.assertEqual(text2.get_attribute("value"), "text is TextArea");
        self.assertEqual(text3.get_attribute("value"), "text is Text");
        self.assertEqual(text4.get_attribute("value"), "text is TextField");
        self.assertEqual(text1.get_attribute("type"), "text");
        self.assertEqual(text2.get_attribute("type"), "text");
        self.assertEqual(text3.get_attribute("type"), "text");
        self.assertEqual(text4.get_attribute("type"), "text");

        self.assertEqual(texts1.get_attribute("value"), None);
        self.assertEqual(texts2.get_attribute("value"), None);
        self.assertEqual(texts3.get_attribute("value"), None);
        self.assertEqual(texts4.get_attribute("value"), None);
        self.assertEqual(texts1.get_attribute("type"), None);
        self.assertEqual(texts2.get_attribute("type"), None);
        self.assertEqual(texts3.get_attribute("type"), None);
        self.assertEqual(texts4.get_attribute("type"), None);

        self.assertEqual(texth1.get_attribute("value"), "●●●●●●●●●●●●●●●●●●●●●●●●");
        self.assertEqual(texth1.get_attribute("type"), "password");
        self.assertEqual(texth4.get_attribute("value"), "●●●●●●●●●●●●●●●●●●●●●●●●");
        self.assertEqual(texth4.get_attribute("type"), "password");

        self.assertEqual(textne1.get_attribute("value"), "");
        self.assertEqual(textne1.get_attribute("type"), "password");
        self.assertEqual(textne4.get_attribute("value"), "");
        self.assertEqual(textne4.get_attribute("type"), "password");

        self.assertEqual(textpe1.get_attribute("value"), "●●●●●●●●●●●●●●●●●●●●●●●●");
        self.assertEqual(textpe1.get_attribute("type"), "password");
        self.assertEqual(textpe4.get_attribute("value"), "●●●●●●●●●●●●●●●●●●●●●●●●");
        self.assertEqual(textpe4.get_attribute("type"), "password");

        self.assertEqual(text1.get_dom_attribute("aria-label"), None);
        self.assertEqual(text2.get_dom_attribute("aria-label"), None);
        self.assertEqual(text3.get_dom_attribute("aria-label"), None);
        self.assertEqual(text4.get_dom_attribute("aria-label"), None);
        self.assertEqual(texth1.get_dom_attribute("aria-label"), None);
        self.assertEqual(texth4.get_dom_attribute("aria-label"), None);
        self.assertEqual(textne1.get_dom_attribute("aria-label"), None);
        self.assertEqual(textne4.get_dom_attribute("aria-label"), None);
        self.assertEqual(textpe1.get_dom_attribute("aria-label"), None);
        self.assertEqual(textpe4.get_dom_attribute("aria-label"), None);

#        time.sleep(3600)
        self.assertEqual(texts1.get_attribute("innerHTML"), "text is static TextInput");
        self.assertEqual(texts2.get_attribute("innerHTML"), "text is static TextArea");
        self.assertEqual(texts3.get_attribute("innerHTML"), "text is static Text");
        self.assertEqual(texts4.get_attribute("innerHTML"), "text is static TextField");

        self.assertEqual(text1DB.get_attribute("innerHTML"), "description is This is the TextInput area");
        self.assertEqual(text2DB.get_attribute("innerHTML"), "description is This is the TextArea area");
        self.assertEqual(text3DB.get_attribute("innerHTML"), "description is This is the Text area");

        self.assertEqual(texts1DB.get_attribute("innerHTML"), "description is This is the static TextInput area");
        self.assertEqual(texts2DB.get_attribute("innerHTML"), "description is This is the static TextArea area");
        self.assertEqual(texts3DB.get_attribute("innerHTML"), "description is This is the static Text area");

        self._driver.execute_script(
            f'''
                instance.kill_QML();
            '''
        )

    def tearDown(self):
        self._driver.quit()

class ScreenPosition(Enum):
    FIXED = auto()
    RELATIVE = auto()
    IN_SCROLL_CONTAINER = auto()

class Screen:
    def __init__(self, driver, positioning=None, x=None, y=None, width=None, height=None, container_width=0, container_height=0, screen_name=None):
        self.driver = driver
        if screen_name is not None:
            screen_information = call_instance_function(self.driver, 'screenInformation')
            if len(screen_information) != 1:
                raise AssertionError('Expecting exactly one screen_information!')
            self.screen_info = screen_information[0]
            self.element = driver.find_element(By.CSS_SELECTOR, f'#test-screen-1')
            return

        if positioning == ScreenPosition.FIXED:
            command = f'initializeScreenWithFixedPosition({x}, {y}, {width}, {height})'
        elif positioning == ScreenPosition.RELATIVE:
            command = f'initializeScreenWithRelativePosition({x}, {y}, {width}, {height})'
        elif positioning == ScreenPosition.IN_SCROLL_CONTAINER:
            command = f'initializeScreenInScrollContainer({container_width}, {container_height}, {x}, {y}, {width}, {height})'
        self.element = self.driver.execute_script(
            f'''
                return testSupport.{command};
            '''
        )
        if positioning == ScreenPosition.IN_SCROLL_CONTAINER:
            self.element = self.element[1]

        screen_information = call_instance_function(
            self.driver, 'screenInformation')
        if len(screen_information) != 1:
            raise AssertionError('Expecting exactly one screen_information!')
        self.screen_info = screen_information[0]

    @property
    def rect(self):
        self.screen_info = call_instance_function(
            self.driver, 'screenInformation')[0]
        geo = self.screen_info['geometry']
        return Rect(geo['x'], geo['y'], geo['width'], geo['height'])

    @property
    def name(self):
        return self.screen_info['name']

    def scroll_to(self):
        ActionChains(self.driver).scroll_to_element(self.element).perform()

    def hit_test_point(self, x, y):
        return self.driver.execute_script(
            f'''
                return testSupport.hitTestPoint({x}, {y}, '{self.element.get_attribute("id")}');
            '''
        )

    def window_stack_at_point(self, x, y):
        return [
            Window(self, element=element) for element in [
                *filter(lambda elem: (elem.get_attribute('id') if elem.get_attribute('id') is not None else '')
                        .startswith('qt-window-'), self.hit_test_point(x, y))]]

    def query_windows(self):
        shadow_container = self.element.find_element(By.CSS_SELECTOR, f'#qt-shadow-container')
        return [
            Window(self, element=element) for element in shadow_container.shadow_root.find_elements(
                    By.CSS_SELECTOR, f'div#{self.name} > div.qt-decorated-window')]

    def find_element(self, method, query):
        shadow_container = self.element.find_element(By.CSS_SELECTOR, f'#qt-shadow-container')
        return shadow_container.shadow_root.find_element(method, query)

    def click(self, element):
        rect = element.rect;
        #ActionChains(self.driver).move_to_element(element).click().perform()
        SELENIUM_IMPRECISION_COMPENSATION = 7
        ActionChains(self.driver).move_to_element(
            element).move_by_offset(-rect["width"] / 2 + SELENIUM_IMPRECISION_COMPENSATION,
                                         -rect["height"] / 2 + SELENIUM_IMPRECISION_COMPENSATION).click().perform()

    def focus(self, element):
        self.driver.execute_script("arguments[0].focus();", element);

    def has_focus(self, element):
        return self.driver.execute_script("return document.activeElement = arguments[0];", element);

class Window:
    def __init__(self, element):
        self.driver = parent.driver

        self.element = element
        self.title = element.find_element(
                By.CSS_SELECTOR, f'.title-bar > .window-name').get_property("textContent")
        information = self.__window_information()
        self.screen = Screen(self.driver, screen_name=information['screen']['name'])

    def __eq__(self, other):
        return self._window_id == other._window_id if isinstance(other, Window) else False

    def __window_information(self):
        information = call_instance_function(self.driver, 'windowInformation')
        return next(filter(lambda e: e['title'] == self.title, information))

    @property
    def rect(self):
        geo = self.__window_information()["geometry"]
        return Rect(geo['x'], geo['y'], geo['width'], geo['height'])

    @property
    def frame_rect(self):
        geo = self.__window_information()["frameGeometry"]
        return Rect(geo['x'], geo['y'], geo['width'], geo['height'])

    @property
    def events(self):
        events = self.driver.execute_script(
            f'''
                return testSupport.events();
            '''
        )
        return [*filter(lambda e: e['windowTitle'] == self.title, events)]

    def set_visible(self, visible):
        info = self.__window_information()
        self.driver.execute_script(
            f'''instance.setWindowVisible({info['id']}, {'true' if visible else 'false'});''')

    def drag(self, handle, direction):
        ActionChains(self.driver)                                                        \
            .move_to_element_with_offset(self.element, *self.at(handle)['offset'])       \
            .click_and_hold()                                                            \
            .move_by_offset(*translate_direction_to_offset(direction))                   \
            .release().perform()

    def maximize(self):
        maximize_button = self.element.find_element(
            By.CSS_SELECTOR, f'.title-bar :nth-child(6)')
        maximize_button.click()

    def at(self, handle):
        """ Returns (window, offset) for given handle on window"""
        width = self.frame_rect.width
        height = self.frame_rect.height

        if handle == Handle.TOP_LEFT:
            offset = (-width/2, -height/2)
        elif handle == Handle.TOP:
            offset = (0, -height/2)
        elif handle == Handle.TOP_RIGHT:
            offset = (width/2, -height/2)
        elif handle == Handle.LEFT:
            offset = (-width/2, 0)
        elif handle == Handle.RIGHT:
            offset = (width/2, 0)
        elif handle == Handle.BOTTOM_LEFT:
            offset = (-width/2, height/2)
        elif handle == Handle.BOTTOM:
            offset = (0, height/2)
        elif handle == Handle.BOTTOM_RIGHT:
            offset = (width/2, height/2)
        elif handle == Handle.TOP_WINDOW_BAR:
            frame_top = self.frame_rect.y
            client_area_top = self.rect.y
            top_frame_bar_width = client_area_top - frame_top
            offset = (0, -height/2 + top_frame_bar_width/2)
        return {'window': self, 'offset': offset}

    @property
    def bounding_box(self):
        raw = self.driver.execute_script("""
            return arguments[0].getBoundingClientRect();
            """, self.element)
        return Rect(raw['x'], raw['y'], raw['width'], raw['height'])

    @property
    def active(self):
        return not self.inactive
        # self.assertFalse('inactive' in window_element.get_attribute(
        #     'class').split(' '), window_element.get_attribute('id'))

    @property
    def inactive(self):
        window_chain = [
            *self.element.find_elements(By.XPATH, "ancestor::div"), self.element]
        return next(filter(lambda elem: 'qt-window' in elem.get_attribute('class').split(' ') and
                           'inactive' in elem.get_attribute(
                               'class').split(' '),
                           window_chain
                           ), None) is not None

    def click(self, x, y):
        rect = self.bounding_box

        SELENIUM_IMPRECISION_COMPENSATION = 2
        ActionChains(self.driver).move_to_element(
            self.element).move_by_offset(-rect.width / 2 + x + SELENIUM_IMPRECISION_COMPENSATION,
                                         -rect.height / 2 + y + SELENIUM_IMPRECISION_COMPENSATION).click().perform()


class TouchDragAction:
    def __init__(self, origin, direction):
        self.origin = origin
        self.direction = direction
        self.step = 2


def perform_touch_drag_actions(actions):
    driver = actions[0].origin['window'].driver
    touch_action_builder = ActionBuilder(driver)
    pointers = [PointerActions(source=touch_action_builder.add_pointer_input(
        POINTER_TOUCH, f'touch_input_{i}')) for i in range(len(actions))]

    for action, pointer in zip(actions, pointers):
        pointer.move_to(
            action.origin['window'].element, *action.origin['offset'])
        pointer.pointer_down(width=10, height=10, pressure=1)
    moves = [translate_direction_to_offset(a.direction) for a in actions]

    def movement_finished():
        for move in moves:
            if move != (0, 0):
                return False
        return True

    def sign(num):
        if num > 0:
            return 1
        elif num < 0:
            return -1
        return 0

    while not movement_finished():
        for i in range(len(actions)):
            pointer = pointers[i]
            move = moves[i]
            step = actions[i].step

            current_move = (
                min(abs(move[0]), step) * sign(move[0]), min(abs(move[1]), step) * sign(move[1]))
            moves[i] = (move[0] - current_move[0], move[1] - current_move[1])
            pointer.move_by(current_move[0],
                            current_move[1], width=10, height=10)
    for pointer in pointers:
        pointer.pointer_up()

    touch_action_builder.perform()


class TouchDragAction:
    def __init__(self, origin, direction):
        self.origin = origin
        self.direction = direction
        self.step = 2


def perform_touch_drag_actions(actions):
    driver = actions[0].origin['window'].driver
    touch_action_builder = ActionBuilder(driver)
    pointers = [PointerActions(source=touch_action_builder.add_pointer_input(
        POINTER_TOUCH, f'touch_input_{i}')) for i in range(len(actions))]

    for action, pointer in zip(actions, pointers):
        pointer.move_to(
            action.origin['window'].element, *action.origin['offset'])
        pointer.pointer_down(width=10, height=10, pressure=1)

    moves = [translate_direction_to_offset(a.direction) for a in actions]

    def movement_finished():
        for move in moves:
            if move != (0, 0):
                return False
        return True

    def sign(num):
        if num > 0:
            return 1
        elif num < 0:
            return -1
        return 0

    while not movement_finished():
        for i in range(len(actions)):
            pointer = pointers[i]
            move = moves[i]
            step = actions[i].step

            current_move = (
                min(abs(move[0]), step) * sign(move[0]), min(abs(move[1]), step) * sign(move[1]))
            moves[i] = (move[0] - current_move[0], move[1] - current_move[1])
            pointer.move_by(current_move[0],
                            current_move[1], width=10, height=10)

    for pointer in pointers:
        pointer.pointer_up()

    touch_action_builder.perform()


def translate_direction_to_offset(direction):
    return (direction.val[1] - direction.val[3], direction.val[2] - direction.val[0])


def call_instance_function(driver, name):
    return driver.execute_script(
        f'''let result;
            window.{name}Callback = data => result = data;
            instance.{name}();
            return eval(result);''')

def call_instance_function_arg(driver, name, arg):
    return driver.execute_script(
        f'''let result;
            window.{name}Callback = data => result = data;
            instance.{name}('{arg}');
            return eval(result);''')

def wait_for_animation_frame(driver):
    driver.execute_script(
        '''
            window.requestAnimationFrame(() => {
                const sync = document.createElement('div');
                sync.id = 'test-sync';
                document.body.appendChild(sync);
            });
        '''
    )
    WebDriverWait(driver, 1).until(
        presence_of_element_located((By.ID, 'test-sync'))
    )
    driver.execute_script(
        '''
            document.body.removeChild(document.body.querySelector('#test-sync'));
        '''
    )

class Direction:
    def __init__(self):
        self.val = (0, 0, 0, 0)

    def __init__(self, north, east, south, west):
        self.val = (north, east, south, west)

    def __add__(self, other):
        return Direction(self.val[0] + other.val[0],
                         self.val[1] + other.val[1],
                         self.val[2] + other.val[2],
                         self.val[3] + other.val[3])


class UP(Direction):
    def __init__(self, step=1):
        self.val = (step, 0, 0, 0)


class RIGHT(Direction):
    def __init__(self, step=1):
        self.val = (0, step, 0, 0)


class DOWN(Direction):
    def __init__(self, step=1):
        self.val = (0, 0, step, 0)


class LEFT(Direction):
    def __init__(self, step=1):
        self.val = (0, 0, 0, step)


class Handle(Enum):
    TOP_LEFT = auto()
    TOP = auto()
    TOP_RIGHT = auto()
    LEFT = auto()
    RIGHT = auto()
    BOTTOM_LEFT = auto()
    BOTTOM = auto()
    BOTTOM_RIGHT = auto()
    TOP_WINDOW_BAR = auto()

class Color:
    def __init__(self, r, g, b):
        self.r = r
        self.g = g
        self.b = b

class Rect:
    def __init__(self, x, y, width, height) -> None:
        self.x = x
        self.y = y
        self.width = width
        self.height = height

    def __str__(self):
        return f'(x: {self.x}, y: {self.y}, width: {self.width}, height: {self.height})'

    @property
    def center(self):
        return self.x + self.width / 2, self.y + self.height / 2,

def assert_colors_equal(color1, color2, msg=None):
    if color1.r != color2.r or color1.g != color2.g or color1.b != color2.b:
        raise AssertionError(f'Colors not equal: \n{color1} \nvs \n{color2}')

def assert_rects_equal(geo1, geo2, msg=None):
    if geo1.x != geo2.x or geo1.y != geo2.y or geo1.width != geo2.width or geo1.height != geo2.height:
        raise AssertionError(f'Rectangles not equal: \n{geo1} \nvs \n{geo2}')

unittest.main()
