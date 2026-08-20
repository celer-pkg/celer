// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcareaallocator_p.h"

QT_BEGIN_NAMESPACE

struct QCAreaAllocatorNode
{
    enum class SplitType { VerticalSplit, HorizontalSplit };

    QCAreaAllocatorNode(QCAreaAllocatorNode *parent);
    ~QCAreaAllocatorNode();
    inline bool isLeaf();

    QCAreaAllocatorNode *parent = nullptr;
    QCAreaAllocatorNode *left = nullptr;
    QCAreaAllocatorNode *right = nullptr;
    int split; // only valid for inner nodes.
    SplitType splitType;
    bool isOccupied = false; // only valid for leaf nodes.
};

QCAreaAllocatorNode::QCAreaAllocatorNode(QCAreaAllocatorNode *parent)
    : parent(parent)
{}

QCAreaAllocatorNode::~QCAreaAllocatorNode()
{
    delete left;
    delete right;
}

bool QCAreaAllocatorNode::isLeaf()
{
    Q_ASSERT((left != nullptr) == (right != nullptr));
    return !left;
}

QCAreaAllocator::QCAreaAllocator(QSize size)
    : m_size(size)
{
    m_root = new QCAreaAllocatorNode(nullptr);
}

QCAreaAllocator::~QCAreaAllocator()
{
    delete m_root;
}

QRect QCAreaAllocator::allocate(QSize size)
{
    QPoint point;
    bool result = allocateInNode(size, point, QRect(QPoint(0, 0), m_size), m_root);
    return result ? QRect(point, size) : QRect();
}

bool QCAreaAllocator::deallocate(QRect rect)
{
    return deallocateInNode(rect.topLeft(), m_root);
}

bool QCAreaAllocator::allocateInNode(
    QSize size, QPoint &result, QRect currentRect, QCAreaAllocatorNode *node)
{
    if (size.width() > currentRect.width() || size.height() > currentRect.height())
        return false;

    if (node->isLeaf()) {
        if (node->isOccupied)
            return false;
        const int maxMargin = 2;
        if (size.width() + maxMargin >= currentRect.width()
            && size.height() + maxMargin >= currentRect.height()) {
            // Snug fit, occupy entire rectangle.
            node->isOccupied = true;
            result = currentRect.topLeft();
            return true;
        }
        // TODO: Reuse nodes.
        // Split node.
        node->left = new QCAreaAllocatorNode(node);
        node->right = new QCAreaAllocatorNode(node);
        QRect splitRect = currentRect;
        if ((currentRect.width() - size.width()) * currentRect.height()
            < (currentRect.height() - size.height()) * currentRect.width()) {
            node->splitType = QCAreaAllocatorNode::SplitType::HorizontalSplit;
            node->split = currentRect.top() + size.height();
            splitRect.setHeight(size.height());
        } else {
            node->splitType = QCAreaAllocatorNode::SplitType::VerticalSplit;
            node->split = currentRect.left() + size.width();
            splitRect.setWidth(size.width());
        }
        return allocateInNode(size, result, splitRect, node->left);
    } else {
        // TODO: avoid unnecessary recursion.
        //  has been split.
        QRect leftRect = currentRect;
        QRect rightRect = currentRect;
        if (node->splitType == QCAreaAllocatorNode::SplitType::HorizontalSplit) {
            leftRect.setHeight(node->split - leftRect.top());
            rightRect.setTop(node->split);
        } else {
            leftRect.setWidth(node->split - leftRect.left());
            rightRect.setLeft(node->split);
        }
        if (allocateInNode(size, result, leftRect, node->left))
            return true;
        if (allocateInNode(size, result, rightRect, node->right))
            return true;
        return false;
    }
}

bool QCAreaAllocator::deallocateInNode(QPoint pos, QCAreaAllocatorNode *node)
{
    while (!node->isLeaf()) {
        //  has been split.
        int cmp = node->splitType == QCAreaAllocatorNode::SplitType::HorizontalSplit ? pos.y()
                                                                                    : pos.x();
        node = cmp < node->split ? node->left : node->right;
    }
    if (!node->isOccupied)
        return false;
    node->isOccupied = false;
    mergeNodeWithNeighbors(node);
    return true;
}

void QCAreaAllocator::mergeNodeWithNeighbors(QCAreaAllocatorNode *node)
{
    bool done = false;
    QCAreaAllocatorNode *parent = nullptr;
    QCAreaAllocatorNode *current = nullptr;
    QCAreaAllocatorNode *sibling;
    while (!done) {
        Q_ASSERT(node->isLeaf());
        Q_ASSERT(!node->isOccupied);
        if (node->parent == nullptr)
            return; // No neighbors.

        QCAreaAllocatorNode::SplitType splitType = QCAreaAllocatorNode::SplitType(
            node->parent->splitType);
        done = true;

        // Merge with left neighbor.
        current = node;
        parent = current->parent;
        while (parent && current == parent->left && parent->splitType == splitType) {
            current = parent;
            parent = parent->parent;
        }

        if (parent && parent->splitType == splitType) {
            Q_ASSERT(current == parent->right);
            Q_ASSERT(parent->left);

            QCAreaAllocatorNode *neighbor = parent->left;
            while (neighbor->right && neighbor->splitType == splitType)
                neighbor = neighbor->right;

            if (neighbor->isLeaf() && neighbor->parent->splitType == splitType
                && !neighbor->isOccupied) {
                // Left neighbor can be merged.
                parent->split = neighbor->parent->split;

                parent = neighbor->parent;
                sibling = neighbor == parent->left ? parent->right : parent->left;
                QCAreaAllocatorNode **nodeRef = &m_root;
                if (parent->parent) {
                    if (parent == parent->parent->left)
                        nodeRef = &parent->parent->left;
                    else
                        nodeRef = &parent->parent->right;
                }
                sibling->parent = parent->parent;
                *nodeRef = sibling;
                parent->left = parent->right = nullptr;
                delete parent;
                delete neighbor;
                done = false;
            }
        }

        // Merge with right neighbor.
        current = node;
        parent = current->parent;
        while (parent && current == parent->right && parent->splitType == splitType) {
            current = parent;
            parent = parent->parent;
        }

        if (parent && parent->splitType == splitType) {
            Q_ASSERT(current == parent->left);
            Q_ASSERT(parent->right);

            QCAreaAllocatorNode *neighbor = parent->right;
            while (neighbor->left && neighbor->splitType == splitType)
                neighbor = neighbor->left;

            if (neighbor->isLeaf() && neighbor->parent->splitType == splitType
                && !neighbor->isOccupied) {
                // Right neighbor can be merged.
                parent->split = neighbor->parent->split;

                parent = neighbor->parent;
                sibling = neighbor == parent->left ? parent->right : parent->left;
                QCAreaAllocatorNode **nodeRef = &m_root;
                if (parent->parent) {
                    if (parent == parent->parent->left)
                        nodeRef = &parent->parent->left;
                    else
                        nodeRef = &parent->parent->right;
                }
                sibling->parent = parent->parent;
                *nodeRef = sibling;
                parent->left = parent->right = nullptr;
                delete parent;
                delete neighbor;
                done = false;
            }
        }
    } // end while (!done)
}

QT_END_NAMESPACE
