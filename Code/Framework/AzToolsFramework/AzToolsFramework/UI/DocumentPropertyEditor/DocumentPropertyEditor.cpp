/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "DocumentPropertyEditor.h"

#include <AzQtComponents/Components/Widgets/ElidingLabel.h>
#include <QLineEdit>
#include <QVBoxLayout>

#include <AzCore/DOM/DomUtils.h>
#include <AzFramework/DocumentPropertyEditor/PropertyEditorNodes.h>
#include <AzToolsFramework/UI/DocumentPropertyEditor/PropertyEditorToolsSystemInterface.h>

namespace AzToolsFramework
{
    // helper method used by both the view and row widgets to empty layouts
    static void DestroyLayoutContents(QLayout* layout)
    {
        while (const auto layoutItem = layout->takeAt(0))
        {
            auto subWidget = layoutItem->widget();
            if (subWidget)
            {
                delete subWidget;
            }
            delete layoutItem;
        }
    }

    QSize DPELayout::sizeHint() const
    {
        int cumulativeWidth = 0;
        int preferredHeight = 0;

        // sizeHint for this horizontal layout is the sum of the preferred widths,
        // and the maximum of the preferred heights
        for (int layoutIndex = 0; layoutIndex < count(); ++layoutIndex)
        {
            QWidget* widgetChild = itemAt(layoutIndex)->widget();
            if (widgetChild)
            {
                auto widgetSizeHint = widgetChild->sizeHint();
                cumulativeWidth += widgetSizeHint.width();
                preferredHeight = qMax(widgetSizeHint.height(), preferredHeight);
            }
        }
        return { cumulativeWidth, preferredHeight };
    }

    QSize DPELayout::minimumSize() const
    {
        int cumulativeWidth = 0;
        int minimumHeight = 0;

        // minimumSize for this horizontal layout is the sum of the min widths,
        // and the maximum of the preferred heights
        for (int layoutIndex = 0; layoutIndex < count(); ++layoutIndex)
        {
            QWidget* widgetChild = itemAt(layoutIndex)->widget();
            if (widgetChild)
            {
                const auto minWidth = widgetChild->minimumSizeHint().width();
                ;
                if (minWidth > 0)
                {
                    cumulativeWidth += minWidth;
                }
                minimumHeight = qMax(widgetChild->sizeHint().height(), minimumHeight);
            }
        }
        return { cumulativeWidth, minimumHeight };
    }

    void DPELayout::setGeometry(const QRect& rect)
    {
        QLayout::setGeometry(rect);

        // for now, just divide evenly horizontally. Later, we'll implement QSplitter-like
        // functionality to allow the user to resize columns within a DPE
        int perItemWidth = rect.width() / count();

        // iterate over the items, laying them left to right
        QRect itemGeometry(rect);

        constexpr int indentSize = 15; // child indent of first item, in pixels
        for (int layoutIndex = 0; layoutIndex < count(); ++layoutIndex)
        {
            itemGeometry.setRight(itemGeometry.left() + perItemWidth);

            // add indent for the first entry
            if (m_depth && layoutIndex == 0)
            {
                itemGeometry.setLeft(m_depth * indentSize);
            }

            itemAt(layoutIndex)->setGeometry(itemGeometry);
            itemGeometry.setLeft(itemGeometry.right() + 1);
        }
    }

    DocumentPropertyEditor* DPELayout::GetDPE() const
    {
        DocumentPropertyEditor* myDPE = qobject_cast<DocumentPropertyEditor*>(parentWidget()->parentWidget());
        AZ_Assert(myDPE, "A DPELayout must be the child of a DPERowWidget, which must be the child of a DocumentPropertyEditor!");
        return myDPE;
    }

    DPERowWidget::DPERowWidget(int depth, QWidget* parentWidget)
        : QWidget(parentWidget)
        , m_depth(depth)
        , m_columnLayout(new DPELayout(depth, this))
    {
        // allow horizontal stretching, but use the vertical size hint exactly
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    DPERowWidget::~DPERowWidget()
    {
        Clear();
    }

    void DPERowWidget::Clear()
    {
        // propertyHandlers own their widgets, so don't destroy them here. Set them free!
        for (auto propertyWidgetIter = m_widgetToPropertyHandler.begin(), endIter = m_widgetToPropertyHandler.end();
             propertyWidgetIter != endIter; ++propertyWidgetIter)
        {
            m_columnLayout->removeWidget(propertyWidgetIter->first);
        }
        m_widgetToPropertyHandler.clear();

        DestroyLayoutContents(m_columnLayout);

        // delete all child rows, this will also remove them from the layout
        for (auto entry : m_domOrderedChildren)
        {
            DPERowWidget* rowChild = qobject_cast<DPERowWidget*>(entry.data());
            if (rowChild)
            {
                rowChild->deleteLater();
            }
        }
    }

    void DPERowWidget::AddChildFromDomValue(const AZ::Dom::Value& childValue, int domIndex)
    {
        // create a child widget from the given DOM value and add it to the correct layout
        auto childType = childValue.GetNodeName();

        if (childType == AZ::Dpe::GetNodeName<AZ::Dpe::Nodes::Row>())
        {
            // determine where to put this new row in the main DPE layout
            auto newRow = new DPERowWidget(m_depth + 1);
            DPERowWidget* priorWidgetInLayout = nullptr;

            // search for an existing row sibling with a lower dom index
            for (int priorWidgetIndex = domIndex - 1; priorWidgetInLayout == nullptr && priorWidgetIndex >= 0; --priorWidgetIndex)
            {
                priorWidgetInLayout = qobject_cast<DPERowWidget*>(m_domOrderedChildren[priorWidgetIndex]);
            }

            // if we found a prior DPERowWidget, put this one after the last of its children,
            // if not, put this new row immediately after its parent -- this
            if (priorWidgetInLayout)
            {
                priorWidgetInLayout = priorWidgetInLayout->GetLastDescendentInLayout();
            }
            else
            {
                priorWidgetInLayout = this;
            }
            m_domOrderedChildren.insert(m_domOrderedChildren.begin() + domIndex, newRow);
            GetDPE()->addAfterWidget(priorWidgetInLayout, newRow);

            // if it's a row, recursively populate the children from the DOM array in the passed value
            newRow->SetValueFromDom(childValue);
        }
        else
        {
            QWidget* addedWidget = nullptr;
            if (childType == AZ::Dpe::GetNodeName<AZ::Dpe::Nodes::Label>())
            {
                auto labelString = childValue[AZ::DocumentPropertyEditor::Nodes::Label::Value.GetName()].GetString();
                addedWidget =
                    new AzQtComponents::ElidingLabel(QString::fromUtf8(labelString.data(), static_cast<int>(labelString.size())), this);
            }
            else if (childType == AZ::Dpe::GetNodeName<AZ::Dpe::Nodes::PropertyEditor>())
            {
                auto dpeSystem = AZ::Interface<AzToolsFramework::PropertyEditorToolsSystemInterface>::Get();
                auto handlerId = dpeSystem->GetPropertyHandlerForNode(childValue);

                // if we found a valid handler, grab its widget to add to the column layout
                if (handlerId)
                {
                    // store, then reference the unique_ptr that will manage the handler's lifetime
                    auto handler = dpeSystem->CreateHandlerInstance(handlerId);
                    handler->SetValueFromDom(childValue);
                    addedWidget = handler->GetWidget();
                    m_widgetToPropertyHandler[addedWidget] = AZStd::move(handler);
                }
                else
                {
                    addedWidget = new QLabel("Missing handler for dom node!");
                }
            }
            else
            {
                AZ_Assert(0, "unknown node type for DPE");
                return;
            }

            addedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            // search for an existing column sibling with a lower dom index
            int priorColumnIndex = -1;
            for (int searchIndex = domIndex - 1; (priorColumnIndex == -1 && searchIndex >= 0); --searchIndex)
            {
                priorColumnIndex = m_columnLayout->indexOf(m_domOrderedChildren[searchIndex]);
            }
            // insert after the found index; even if nothing were found and priorIndex is still -1,
            // still insert one after it, at position 0
            m_columnLayout->insertWidget(priorColumnIndex + 1, addedWidget);
            m_domOrderedChildren.insert(m_domOrderedChildren.begin() + domIndex, addedWidget);
        }
    }

    void DPERowWidget::SetValueFromDom(const AZ::Dom::Value& domArray)
    {
        Clear();

        // populate all direct children of this row
        for (size_t arrayIndex = 0, numIndices = domArray.ArraySize(); arrayIndex < numIndices; ++arrayIndex)
        {
            auto& childValue = domArray[arrayIndex];
            AddChildFromDomValue(childValue, static_cast<int>(arrayIndex));
        }
    }

    void DPERowWidget::HandleOperationAtPath(const AZ::Dom::PatchOperation& domOperation, size_t pathIndex)
    {
        const auto& fullPath = domOperation.GetDestinationPath();
        auto pathEntry = fullPath[pathIndex];
        AZ_Assert(pathEntry.IsIndex() || pathEntry.IsEndOfArray(), "the direct children of a row must be referenced by index");
        const int childCount = static_cast<int>(m_domOrderedChildren.size());

        // if we're on the last entry in the path, this row widget is the direct owner
        if (pathIndex == fullPath.Size() - 1)
        {
            int childIndex = 0;
            if (pathEntry.IsIndex())
            {
                // remove and replace operations must match an existing index. Add operations can be one past the current end.
                AZ_Assert(
                    (domOperation.GetType() == AZ::Dom::PatchOperation::Type::Add ? childIndex <= childCount : childIndex < childCount),
                    "patch index is beyond the array bounds!");

                childIndex = static_cast<int>(pathEntry.GetIndex());
            }
            else if (domOperation.GetType() == AZ::Dom::PatchOperation::Type::Add)
            {
                childIndex = childCount;
            }
            else // must be IsEndOfArray and a replace or remove, use the last existing index
            {
                childIndex = childCount - 1;
            }

            // if this is a remove or replace, remove the existing entry first,
            // then, if this is a replace or add, add the new entry
            if (domOperation.GetType() == AZ::Dom::PatchOperation::Type::Remove ||
                domOperation.GetType() == AZ::Dom::PatchOperation::Type::Replace)
            {
                const auto childIterator = m_domOrderedChildren.begin() + childIndex;
                delete *childIterator; // deleting the widget also automatically removes it from the layout
                m_domOrderedChildren.erase(childIterator);
            }

            if (domOperation.GetType() == AZ::Dom::PatchOperation::Type::Replace ||
                domOperation.GetType() == AZ::Dom::PatchOperation::Type::Add)
            {
                AddChildFromDomValue(domOperation.GetValue(), static_cast<int>(childIndex));
            }
        }
        else // not the direct owner of the entry to patch
        {
            // find the next widget in the path and delegate the operation to them
            const int childIndex = (pathEntry.IsIndex() ? static_cast<int>(pathEntry.GetIndex()) : childCount - 1);
            AZ_Assert(childIndex <= childCount, "DPE: Patch failed to apply, invalid child index specified");
            if (childIndex > childCount)
            {
                return;
            }

            QWidget* childWidget = m_domOrderedChildren[childIndex];
            DPERowWidget* widgetAsDPERow = qobject_cast<DPERowWidget*>(childWidget);
            if (widgetAsDPERow)
            {
                // child is a DPERowWidget, pass patch processing to it
                widgetAsDPERow->HandleOperationAtPath(domOperation, pathIndex + 1);
            }
            else // child must be a label or a PropertyEditor
            {
                // pare down the path to this node, then look up and set the value from the DOM
                auto subPath = fullPath;
                for (size_t pathEntryIndex = fullPath.size() - 1; pathEntryIndex > pathIndex; --pathEntryIndex)
                {
                    subPath.Pop();
                }
                const auto valueAtSubPath = GetDPE()->GetAdapter()->GetContents()[subPath];

                // check if it's a PropertyHandler; if it is, just set it from the DOM directly
                auto foundEntry = m_widgetToPropertyHandler.find(childWidget);
                if (foundEntry != m_widgetToPropertyHandler.end())
                {
                    foundEntry->second->SetValueFromDom(valueAtSubPath);
                }
                else
                {
                    QLabel* changedLabel = qobject_cast<QLabel*>(childWidget);
                    AZ_Assert(changedLabel, "not a label, unknown widget discovered!");
                    if (changedLabel)
                    {
                        auto labelString = valueAtSubPath[AZ::DocumentPropertyEditor::Nodes::Label::Value.GetName()].GetString();
                        changedLabel->setText(QString::fromUtf8(labelString.data()));
                    }
                }
            }
        }
    }

    DocumentPropertyEditor* DPERowWidget::GetDPE()
    {
        DocumentPropertyEditor* theDPE = nullptr;
        QWidget* ancestorWidget = parentWidget();
        while (ancestorWidget && !theDPE)
        {
            theDPE = qobject_cast<DocumentPropertyEditor*>(ancestorWidget);
            ancestorWidget = ancestorWidget->parentWidget();
        }
        AZ_Assert(theDPE, "the top level widget in any DPE hierarchy must be the DocumentPropertyEditor itself!");
        return theDPE;
    }

    DPERowWidget* DPERowWidget::GetLastDescendentInLayout()
    {
        DPERowWidget* lastDescendent = nullptr;
        for (auto childIter = m_domOrderedChildren.rbegin(); (lastDescendent == nullptr && childIter != m_domOrderedChildren.rend());
             ++childIter)
        {
            lastDescendent = qobject_cast<DPERowWidget*>(childIter->data());
        }
        if (lastDescendent)
        {
            lastDescendent = lastDescendent->GetLastDescendentInLayout();
        }
        else
        {
            // didn't find any relevant children, this row widget is the last descendent
            lastDescendent = this;
        }
        return lastDescendent;
    }

    DocumentPropertyEditor::DocumentPropertyEditor(QWidget* parentWidget)
        : QFrame(parentWidget)
    {
        m_layout = new QVBoxLayout(this);
    }

    DocumentPropertyEditor::~DocumentPropertyEditor()
    {
        DestroyLayoutContents(GetVerticalLayout());
    }

    void DocumentPropertyEditor::SetAdapter(AZ::DocumentPropertyEditor::DocumentAdapter* theAdapter)
    {
        m_adapter = theAdapter;
        m_resetHandler = AZ::DocumentPropertyEditor::DocumentAdapter::ResetEvent::Handler(
            [this]()
            {
                this->HandleReset();
            });
        m_adapter->ConnectResetHandler(m_resetHandler);

        m_changedHandler = AZ::DocumentPropertyEditor::DocumentAdapter::ChangedEvent::Handler(
            [this](const AZ::Dom::Patch& patch)
            {
                this->HandleDomChange(patch);
            });
        m_adapter->ConnectChangedHandler(m_changedHandler);

        // populate the view from the full adapter contents, just like a reset
        HandleReset();
    }

    void DocumentPropertyEditor::addAfterWidget(QWidget* precursor, QWidget* widgetToAdd)
    {
        int foundIndex = m_layout->indexOf(precursor);
        if (foundIndex >= 0)
        {
            m_layout->insertWidget(foundIndex + 1, widgetToAdd);
        }
    }

    QVBoxLayout* DocumentPropertyEditor::GetVerticalLayout()
    {
        return m_layout;
    }

    void DocumentPropertyEditor::AddRowFromValue(const AZ::Dom::Value& domValue, int rowIndex)
    {
        auto newRow = new DPERowWidget(0, this);
        m_layout->insertWidget(rowIndex, newRow);
        newRow->SetValueFromDom(domValue);
    }

    void DocumentPropertyEditor::HandleReset()
    {
        // clear any pre-existing DPERowWidgets
        DestroyLayoutContents(m_layout);

        auto topContents = m_adapter->GetContents();

        for (size_t arrayIndex = 0, numIndices = topContents.ArraySize(); arrayIndex < numIndices; ++arrayIndex)
        {
            auto& rowValue = topContents[arrayIndex];
            auto domName = rowValue.GetNodeName().GetStringView();
            const bool isRow = (domName == AZ::DocumentPropertyEditor::Nodes::Row::Name);
            AZ_Assert(isRow, "adapters must only have rows as direct children!");

            if (isRow)
            {
                AddRowFromValue(rowValue, static_cast<int>(arrayIndex));
            }
        }
        m_layout->addStretch();
    }
    void DocumentPropertyEditor::HandleDomChange(const AZ::Dom::Patch& patch)
    {
        for (auto operationIterator = patch.begin(), endIterator = patch.end(); operationIterator != endIterator; ++operationIterator)
        {
            const auto& patchPath = operationIterator->GetDestinationPath();
            auto firstAddressEntry = patchPath[0];

            AZ_Assert(
                firstAddressEntry.IsIndex() || firstAddressEntry.IsEndOfArray(),
                "first entry in a DPE patch must be the index of the first row");
            auto rowIndex = (firstAddressEntry.IsIndex() ? firstAddressEntry.GetIndex() : m_layout->count());
            AZ_Assert(
                rowIndex < m_layout->count() ||
                    (rowIndex <= m_layout->count() && operationIterator->GetType() == AZ::Dom::PatchOperation::Type::Add),
                "received a patch for a row that doesn't exist");

            // if the patch points at our root, this operation is for the top level layout
            if (patchPath.IsEmpty())
            {
                if (operationIterator->GetType() == AZ::Dom::PatchOperation::Type::Add)
                {
                    AddRowFromValue(operationIterator->GetValue(), static_cast<int>(rowIndex));
                }
                else
                {
                    auto rowWidget =
                        static_cast<DPERowWidget*>(GetVerticalLayout()->itemAt(static_cast<int>(firstAddressEntry.GetIndex()))->widget());
                    if (operationIterator->GetType() == AZ::Dom::PatchOperation::Type::Replace)
                    {
                        rowWidget->SetValueFromDom(operationIterator->GetValue());
                    }
                    else if (operationIterator->GetType() == AZ::Dom::PatchOperation::Type::Remove)
                    {
                        m_layout->removeWidget(rowWidget);
                        delete rowWidget;
                    }
                }
            }
            else
            {
                // delegate the action th the rowWidget, which will, in turn, delegate to the next row in the path, if available
                auto rowWidget = static_cast<DPERowWidget*>(m_layout->itemAt(static_cast<int>(firstAddressEntry.GetIndex()))->widget());

                constexpr size_t pathDepth = 1; // top level has been handled, start the next operation at path depth 1
                rowWidget->HandleOperationAtPath(*operationIterator, pathDepth);
            }
        }
    }

} // namespace AzToolsFramework
