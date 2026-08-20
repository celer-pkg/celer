// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtQuick3DUtils/private/qssginvasivelinkedlist_p.h>

struct NodeItem
{
    NodeItem() = default;
    NodeItem(int v, NodeItem *p, NodeItem *n) : value(v), prev(p), next(n) {}
    int value = -1;
    NodeItem *prev = nullptr;
    NodeItem *next = nullptr;
};

using SingleLinkedList = QSSGInvasiveSingleLinkedList<NodeItem, &NodeItem::next>;
using LinkedList = QSSGInvasiveLinkedList<NodeItem, &NodeItem::prev, &NodeItem::next>;

class invasivelist : public QObject
{
    Q_OBJECT

public:
    invasivelist() = default;
    ~invasivelist() = default;

private slots:
    void test_empty();
    void test_push_backSll();
    void test_push_frontSll();
    void test_push_backLl();
    void test_push_frontLl();
    void test_removeSll();
    void test_removeLl();
    void test_insert_unsafe();
    void test_insert_before();
    void test_insert_after();
    void test_copy_slList();
    void test_copy_lList();
    void test_iteratorSll();
    void test_iteratorLl();
    void test_sublistSll();
    void test_insertBadNodeLl();
    void test_clearSll();
    void test_removeAllSll();
    void test_clearLl();
    void test_removeAllLl();
};

void invasivelist::test_empty()
{
    SingleLinkedList sll;
    QVERIFY(sll.isEmpty());
    LinkedList ll;
    QVERIFY(ll.isEmpty());
    NodeItem node;
    QCOMPARE(node.next, nullptr);
    QCOMPARE(node.prev, nullptr);

    sll.push_back(node);
    QVERIFY(!sll.isEmpty());
    sll.remove(node);
    // list should be empty and the node "links" reset
    QVERIFY(sll.isEmpty());
    QCOMPARE(node.next, nullptr);
    QCOMPARE(node.prev, nullptr);

    ll.push_back(node);
    QVERIFY(!ll.isEmpty());
    ll.remove(node);
    // list should be empty and the node "links" reset
    QVERIFY(ll.isEmpty());
    QCOMPARE(node.next, nullptr);
    QCOMPARE(node.prev, nullptr);
}

void invasivelist::test_push_backSll()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    SingleLinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    int lastValue = -1;
    int count = 0;
    for (const auto &item : std::as_const(list)) {
        QVERIFY(item.value > lastValue);
        lastValue = item.value;
        ++count;
    }

    QCOMPARE(lastValue, itemCount - 1);
    QCOMPARE(count, itemCount);
}

void invasivelist::test_push_frontSll()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    SingleLinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_front(items[i]);
    }

    int lastValue = 100;
    int count = 0;
    for (const auto &item : std::as_const(list)) {
        QVERIFY(item.value < lastValue);
        lastValue = item.value;
        ++count;
    }

    QCOMPARE(lastValue, 0);
    QCOMPARE(count, itemCount);
}

void invasivelist::test_push_backLl()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    LinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    int lastValue = -1;
    int count = 0;
    for (const auto &item : std::as_const(list)) {
        QVERIFY(item.value > lastValue);
        lastValue = item.value;
        ++count;
    }

    QCOMPARE(lastValue, itemCount - 1);
    QCOMPARE(count, itemCount);
}

void invasivelist::test_push_frontLl()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    LinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_front(items[i]);
    }

    int lastValue = 100;
    int count = 0;
    for (const auto &item : std::as_const(list)) {
        QVERIFY(item.value < lastValue);
        lastValue = item.value;
        ++count;
    }

    QCOMPARE(lastValue, 0);
    QCOMPARE(count, itemCount);
}

void invasivelist::test_removeSll()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    SingleLinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    // Remove an item somewhere in the middel of the list
    QCOMPARE(items[49].next, &items[50]);
    QCOMPARE(items[49].prev, nullptr);
    QCOMPARE(items[48].next, &items[49]);
    list.remove(items[49]);
    QCOMPARE(items[49].next, nullptr);
    QCOMPARE(items[49].prev, nullptr);
    QCOMPARE(items[48].next, &items[50]);

    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &items[1]);
    list.remove(items[0]);
    QCOMPARE(items[0].next, nullptr);
    QCOMPARE(items[0].prev, nullptr);

    list.push_front(items[0]);
    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &items[1]);

    // Remove all
    for (int i = 0; i != itemCount; ++i)
        list.remove(items[i]);

    QVERIFY(list.isEmpty());

    for (int i = 0; i != itemCount; ++i) {
        QCOMPARE(items[i].next, nullptr);
        QCOMPARE(items[i].prev, nullptr);
    }
}

void invasivelist::test_removeLl()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    LinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    // Remove first
    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &items[1]);
    QCOMPARE(items[1].prev, &items[0]);
    QCOMPARE(items[1].next, &items[2]);
    list.remove(items[0]);
    QCOMPARE(items[0].next, nullptr);
    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[1].next, &items[2]);
    QCOMPARE(items[1].prev, nullptr);

    list.push_front(items[0]);
    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &items[1]);

    // Remove !first
    QCOMPARE(items[49].next, &items[50]);
    QCOMPARE(items[51].prev, &items[50]);
    list.remove(items[50]);
    QCOMPARE(items[50].next, nullptr);
    QCOMPARE(items[50].prev, nullptr);
    QCOMPARE(items[49].next, &items[51]);
    QCOMPARE(items[51].prev, &items[49]);

    list.insert_after(items[49], items[50]);

    // Remove all
    for (int i = 0; i != itemCount; ++i)
        list.remove(items[i]);

    QVERIFY(list.isEmpty());

    for (int i = 0; i != itemCount; ++i) {
        QCOMPARE(items[i].next, nullptr);
        QCOMPARE(items[i].prev, nullptr);
    }
}

// Shared between the two, so we don't need to duplcate these
void invasivelist::test_insert_unsafe()
{
    {
        const int itemCount = 2;
        NodeItem items[itemCount] { NodeItem { 0, nullptr, nullptr },
                                    NodeItem { 2, nullptr, nullptr } };
        LinkedList list;
        list.push_back(items[0]);
        list.push_back(items[1]);

        QCOMPARE(items[0].prev, nullptr);
        QCOMPARE(items[0].next, &items[1]);
        QCOMPARE(items[1].prev, &items[0]);
        QCOMPARE(items[1].next, nullptr);

        NodeItem middle { 1, nullptr, nullptr };
        list.insert_unsafe(&items[0], &items[1], middle);
        QCOMPARE(items[0].prev, nullptr);
        QCOMPARE(items[0].next, &middle);
        QCOMPARE(middle.prev, &items[0]);
        QCOMPARE(middle.next, &items[1]);
        QCOMPARE(items[1].prev, &middle);
        QCOMPARE(items[1].next, nullptr);
    }
    {
        const int itemCount = 3;
        NodeItem items[itemCount] { NodeItem { 0, nullptr, nullptr },
                                    NodeItem { -1, nullptr, nullptr },
                                    NodeItem { 2, nullptr, nullptr } };
        LinkedList list;
        list.push_back(items[0]);
        list.push_back(items[1]);
        list.push_back(items[2]);

        QCOMPARE(items[0].prev, nullptr);
        QCOMPARE(items[0].next, &items[1]);
        QCOMPARE(items[1].prev, &items[0]);
        QCOMPARE(items[1].next, &items[2]);
        QCOMPARE(items[2].prev, &items[1]);
        QCOMPARE(items[2].next, nullptr);

        NodeItem middle { 1, nullptr, nullptr };
        list.insert_unsafe(&items[0], &items[2], middle);
        QCOMPARE(items[0].prev, nullptr);
        QCOMPARE(items[0].next, &middle);
        QCOMPARE(middle.prev, &items[0]);
        QCOMPARE(middle.next, &items[2]);
        QCOMPARE(items[2].prev, &middle);
        QCOMPARE(items[2].next, nullptr);
        // item[1] is now danling, but the tail and head is
        // still pointing to the old items
        // (this is inteded, so test that it still works)
        QCOMPARE(items[1].prev, &items[0]);
        QCOMPARE(items[1].next, &items[2]);
    }
}

void invasivelist::test_insert_before()
{
    const int itemCount = 2;
    NodeItem items[itemCount] { NodeItem { 1, nullptr, nullptr },
                                NodeItem { 4, nullptr, nullptr } };
    LinkedList list;
    list.push_back(items[0]);
    list.push_back(items[1]);

    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &items[1]);
    QCOMPARE(items[1].prev, &items[0]);
    QCOMPARE(items[1].next, nullptr);

    NodeItem newItems[] { NodeItem { 0, nullptr, nullptr },
                          NodeItem { 2, nullptr, nullptr },
                          NodeItem { 3, nullptr, nullptr }};

    list.insert_before(items[0], newItems[0]);
    QCOMPARE(newItems[0].prev, nullptr);
    QCOMPARE(newItems[0].next, &items[0]);
    QCOMPARE(items[0].prev, &newItems[0]);
    QCOMPARE(items[0].next, &items[1]);
    QCOMPARE(items[1].prev, &items[0]);
    QCOMPARE(items[1].next, nullptr);

    list.insert_before(items[1], newItems[1]);
    QCOMPARE(newItems[0].prev, nullptr);
    QCOMPARE(newItems[0].next, &items[0]);
    QCOMPARE(items[0].prev, &newItems[0]);
    QCOMPARE(items[0].next, &newItems[1]);
    QCOMPARE(newItems[1].prev, &items[0]);
    QCOMPARE(newItems[1].next, &items[1]);
    QCOMPARE(items[1].prev, &newItems[1]);
    QCOMPARE(items[1].next, nullptr);

    list.insert_before(items[1], newItems[2]);
    QCOMPARE(newItems[0].prev, nullptr);
    QCOMPARE(newItems[0].next, &items[0]);
    QCOMPARE(items[0].prev, &newItems[0]);
    QCOMPARE(items[0].next, &newItems[1]);
    QCOMPARE(newItems[1].prev, &items[0]);
    QCOMPARE(newItems[1].next, &newItems[2]);
    QCOMPARE(newItems[2].prev, &newItems[1]);
    QCOMPARE(newItems[2].next, &items[1]);
    QCOMPARE(items[1].prev, &newItems[2]);
    QCOMPARE(items[1].next, nullptr);
}

void invasivelist::test_insert_after()
{
    const int itemCount = 2;
    NodeItem items[itemCount] { NodeItem { 0, nullptr, nullptr },
                                NodeItem { 3, nullptr, nullptr } };
    LinkedList list;
    list.push_back(items[0]);
    list.push_back(items[1]);

    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &items[1]);
    QCOMPARE(items[1].prev, &items[0]);
    QCOMPARE(items[1].next, nullptr);

    NodeItem newItems[] { NodeItem { 1, nullptr, nullptr },
                          NodeItem { 2, nullptr, nullptr },
                          NodeItem { 4, nullptr, nullptr }};

    list.insert_after(items[0], newItems[0]);
    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &newItems[0]);
    QCOMPARE(newItems[0].prev, &items[0]);
    QCOMPARE(newItems[0].next, &items[1]);
    QCOMPARE(items[1].prev, &newItems[0]);
    QCOMPARE(items[1].next, nullptr);

    list.insert_after(newItems[0], newItems[1]);
    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &newItems[0]);
    QCOMPARE(newItems[0].prev, &items[0]);
    QCOMPARE(newItems[0].next, &newItems[1]);
    QCOMPARE(newItems[1].prev, &newItems[0]);
    QCOMPARE(newItems[1].next, &items[1]);
    QCOMPARE(items[1].prev, &newItems[1]);
    QCOMPARE(items[1].next, nullptr);

    list.insert_after(items[1], newItems[2]);
    QCOMPARE(items[0].prev, nullptr);
    QCOMPARE(items[0].next, &newItems[0]);
    QCOMPARE(newItems[0].prev, &items[0]);
    QCOMPARE(newItems[0].next, &newItems[1]);
    QCOMPARE(newItems[1].prev, &newItems[0]);
    QCOMPARE(newItems[1].next, &items[1]);
    QCOMPARE(items[1].prev, &newItems[1]);
    QCOMPARE(items[1].next, &newItems[2]);
    QCOMPARE(newItems[2].prev, &items[1]);
    QCOMPARE(newItems[2].next, nullptr);
}

void invasivelist::test_copy_slList()
{
    const int itemCount = 2;
    NodeItem items[itemCount] { NodeItem { 0, nullptr, nullptr },
                                NodeItem { 1, nullptr, nullptr } };

    SingleLinkedList l1, l2;
    QVERIFY(l1.isEmpty());
    QVERIFY(l2.isEmpty());
    l1.push_back(items[0]);
    l1.push_back(items[1]);
    QVERIFY(!l1.isEmpty());
    QVERIFY(l2.isEmpty());
    l2 = l1;
    QVERIFY(!l1.isEmpty());
    QVERIFY(!l2.isEmpty());

    for (auto it1 = l1.begin(), it2 = l2.begin(); it1 != l1.end(); ++it1, ++it2)
        QCOMPARE(it2, it1);
}

void invasivelist::test_copy_lList()
{
    const int itemCount = 2;
    NodeItem items[itemCount] { NodeItem { 0, nullptr, nullptr },
                                NodeItem { 1, nullptr, nullptr } };

    LinkedList l1, l2;
    QVERIFY(l1.isEmpty());
    QVERIFY(l2.isEmpty());
    l1.push_back(items[0]);
    l1.push_back(items[1]);
    QVERIFY(!l1.isEmpty());
    QVERIFY(l2.isEmpty());
    l2 = l1;
    QVERIFY(!l1.isEmpty());
    QVERIFY(!l2.isEmpty());

    for (auto it1 = l1.begin(), it2 = l2.begin(); it1 != l1.end(); ++it1, ++it2)
        QCOMPARE(it2, it1);
}

void invasivelist::test_iteratorSll()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    SingleLinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    {
        auto it = list.begin();
        const auto end = list.end();
        int lastValue = -1;
        for (; it != end; ++it) {
            QVERIFY(it->value > lastValue);
            lastValue = it->value;
        }
        QCOMPARE(lastValue, itemCount - 1);
    }
}

void invasivelist::test_iteratorLl()
{
    const int itemCount = 100;
    NodeItem items[itemCount];

    LinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    {
        auto it = list.begin();
        const auto end = list.end();
        int lastValue = -1;
        for (; it != end; ++it) {
            QVERIFY(it->value > lastValue);
            lastValue = it->value;
        }
        QCOMPARE(lastValue, itemCount - 1);
    }

    {
        auto rit = list.rbegin();
        const auto rend = list.rend();
        int lastValue = itemCount;
        for (; rit != rend; ++rit) {
            QVERIFY(rit->value < lastValue);
            lastValue = rit->value;
        }
        QCOMPARE(lastValue, 0);
    }
}

void invasivelist::test_sublistSll()
{
    const int itemCount = 6;
    NodeItem items[itemCount];

    SingleLinkedList l1;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        l1.push_back(items[i]);
    }

    int lastValue = -1;
    int count = 0;
    for (const auto &item : std::as_const(l1)) {
        QVERIFY(item.value > lastValue);
        lastValue = item.value;
        ++count;
    }

    QCOMPARE(lastValue, itemCount - 1);
    QCOMPARE(count, itemCount);

    {
        const int badNodeLastValue = 123;
        { // push_back
            NodeItem badNode;
            badNode.value = badNodeLastValue;
            badNode.next = &items[0];
            badNode.prev = &items[0];
            l1.push_back(badNode);

            lastValue = -1;
            count = 0;
            for (const auto &item : std::as_const(l1)) {
                lastValue = item.value;
                ++count;
            }

            QCOMPARE(lastValue, badNodeLastValue);
            QCOMPARE(count, itemCount + 1);
        }
    }
}

void invasivelist::test_insertBadNodeLl()
{
    const int itemCount = 6;
    NodeItem items[itemCount];

    LinkedList l1;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        l1.push_back(items[i]);
    }

    int lastValue = -1;
    int count = 0;
    for (const auto &item : std::as_const(l1)) {
        QVERIFY(item.value > lastValue);
        lastValue = item.value;
        ++count;
    }

    QCOMPARE(lastValue, itemCount - 1);
    QCOMPARE(count, itemCount);

    {
        const int badNodeLastValue = 123;
        { // push_back
            NodeItem badNode;
            badNode.value = badNodeLastValue;
            badNode.next = &items[0];
            badNode.prev = &items[0];
            l1.push_back(badNode);

            // Forwarnd
            lastValue = -1;
            count = 0;
            for (const auto &item : std::as_const(l1)) {
                lastValue = item.value;
                ++count;
            }

            QCOMPARE(lastValue, badNodeLastValue);
            QCOMPARE(count, itemCount + 1);

            // Reverse
            lastValue = -1;
            count = 0;
            for (auto rit = l1.rbegin(), rend = l1.rend(); rit != rend; ++rit) {
                lastValue = (*rit).value;
                ++count;
            }

            QCOMPARE(lastValue,  items[0].value);
            QCOMPARE(count, itemCount + 1);

            l1.remove(badNode);
            QCOMPARE(badNode.next, nullptr);
            QCOMPARE(badNode.prev, nullptr);
        }

        { // push_front
            NodeItem badNode;
            badNode.value = badNodeLastValue;
            badNode.next = &items[0];
            badNode.prev = &items[0];
            l1.push_front(badNode);

            // Forwarnd
            lastValue = -1;
            count = 0;
            for (const auto &item : std::as_const(l1)) {
                lastValue = item.value;
                ++count;
            }

            QCOMPARE(lastValue, items[itemCount - 1].value);
            QCOMPARE(count, itemCount + 1);

            // Reverse
            lastValue = -1;
            count = 0;
            for (auto rit = l1.rbegin(), rend = l1.rend(); rit != rend; ++rit) {
                lastValue = (*rit).value;
                ++count;
            }

            QCOMPARE(lastValue,  badNodeLastValue);
            QCOMPARE(count, itemCount + 1);

            l1.remove(badNode);
            QCOMPARE(badNode.next, nullptr);
            QCOMPARE(badNode.prev, nullptr);
        }

    }
}

void invasivelist::test_clearSll()
{
    const int itemCount = 3;
    NodeItem items[itemCount];

    SingleLinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    QVERIFY(!list.isEmpty());
    QCOMPARE(items[1].next, &items[2]);
    list.clear();

    // The only change should that the list is empty.
    QVERIFY(list.isEmpty());
    QCOMPARE(items[1].next, &items[2]);
}

void invasivelist::test_removeAllSll()
{
    const int itemCount = 10;
    NodeItem items[itemCount];

    SingleLinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    QVERIFY(!list.isEmpty());
    QCOMPARE(items[1].next, &items[2]);
    list.removeAll();

    // The list should be empty and all the nodes should have their links updated to null.
    QVERIFY(list.isEmpty());
    for (int i = 0; i != itemCount; ++i)
        QCOMPARE(items[i].next, nullptr);
}

void invasivelist::test_clearLl()
{
    const int itemCount = 3;
    NodeItem items[itemCount];

    LinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    QVERIFY(!list.isEmpty());
    QCOMPARE(items[1].prev, &items[0]);
    QCOMPARE(items[1].next, &items[2]);
    list.clear();

    // The only change should that the list is empty.
    QVERIFY(list.isEmpty());
    QCOMPARE(items[1].prev, &items[0]);
    QCOMPARE(items[1].next, &items[2]);
}

void invasivelist::test_removeAllLl()
{
    const int itemCount = 10;
    NodeItem items[itemCount];

    LinkedList list;
    for (int i = 0; i != itemCount; ++i) {
        items[i].value = i;
        list.push_back(items[i]);
    }

    QVERIFY(!list.isEmpty());
    QCOMPARE(items[1].prev, &items[0]);
    QCOMPARE(items[1].next, &items[2]);
    list.removeAll();

    // The list should be empty and all the nodes should have their links updated to null.
    QVERIFY(list.isEmpty());
    for (int i = 0; i != itemCount; ++i) {
        QCOMPARE(items[i].prev, nullptr);
        QCOMPARE(items[i].next, nullptr);
    }
}

QTEST_APPLESS_MAIN(invasivelist)

#include "tst_invasivelist.moc"
