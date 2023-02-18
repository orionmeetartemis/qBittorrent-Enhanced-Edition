/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2016  Vladimir Golovnev <glassez@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "categoryfilterwidget.h"

#include <QMenu>

#include "base/bittorrent/session.h"
#include "base/global.h"
#include "categoryfiltermodel.h"
#include "categoryfilterproxymodel.h"
#include "torrentcategorydialog.h"
#include "uithememanager.h"
#include "utils.h"

namespace
{
    QString getCategoryFilter(const CategoryFilterProxyModel *const model, const QModelIndex &index)
    {
        QString categoryFilter; // Defaults to All
        if (index.isValid())
        {
            if (!index.parent().isValid() && (index.row() == 1))
                categoryFilter = u""_qs; // Uncategorized
            else if (index.parent().isValid() || (index.row() > 1))
                categoryFilter = model->categoryName(index);
        }

        return categoryFilter;
    }
}

CategoryFilterWidget::CategoryFilterWidget(QWidget *parent)
    : QTreeView(parent)
{
    auto *proxyModel = new CategoryFilterProxyModel(this);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setSourceModel(new CategoryFilterModel(this));
    setModel(proxyModel);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setUniformRowHeights(true);
    setHeaderHidden(true);
    setIconSize(Utils::Gui::smallIconSize());
#ifdef Q_OS_MACOS
    setAttribute(Qt::WA_MacShowFocusRect, false);
#endif
    m_defaultIndentation = indentation();
    if (!BitTorrent::Session::instance()->isSubcategoriesEnabled())
        setIndentation(0);
    setContextMenuPolicy(Qt::CustomContextMenu);
    sortByColumn(0, Qt::AscendingOrder);
    setCurrentIndex(model()->index(0, 0));

    connect(this, &QTreeView::collapsed, this, &CategoryFilterWidget::callUpdateGeometry);
    connect(this, &QTreeView::expanded, this, &CategoryFilterWidget::callUpdateGeometry);
    connect(this, &QWidget::customContextMenuRequested, this, &CategoryFilterWidget::showMenu);
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged
            , this, &CategoryFilterWidget::onCurrentRowChanged);
    connect(model(), &QAbstractItemModel::modelReset, this, &CategoryFilterWidget::callUpdateGeometry);
}

QString CategoryFilterWidget::currentCategory() const
{
    QModelIndex current;
    const auto selectedRows = selectionModel()->selectedRows();
    if (!selectedRows.isEmpty())
        current = selectedRows.first();

    return getCategoryFilter(static_cast<CategoryFilterProxyModel *>(model()), current);
}

void CategoryFilterWidget::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    emit categoryChanged(getCategoryFilter(static_cast<CategoryFilterProxyModel *>(model()), current));
}

void CategoryFilterWidget::showMenu()
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction(UIThemeManager::instance()->getIcon(u"list-add"_qs), tr("Add category...")
        , this, &CategoryFilterWidget::addCategory);

    const auto selectedRows = selectionModel()->selectedRows();
    if (!selectedRows.empty() && !CategoryFilterModel::isSpecialItem(selectedRows.first()))
    {
        if (BitTorrent::Session::instance()->isSubcategoriesEnabled())
        {
            menu->addAction(UIThemeManager::instance()->getIcon(u"list-add"_qs), tr("Add subcategory...")
                , this, &CategoryFilterWidget::addSubcategory);
        }

        menu->addAction(UIThemeManager::instance()->getIcon(u"edit-rename"_qs, u"document-edit"_qs), tr("Edit category...")
            , this, &CategoryFilterWidget::editCategory);
        menu->addAction(UIThemeManager::instance()->getIcon(u"edit-clear"_qs, u"list-remove"_qs), tr("Remove category")
            , this, &CategoryFilterWidget::removeCategory);
    }

    menu->addAction(UIThemeManager::instance()->getIcon(u"edit-clear"_qs, u"list-remove"_qs), tr("Remove unused categories")
        , this, &CategoryFilterWidget::removeUnusedCategories);
    menu->addSeparator();
    menu->addAction(UIThemeManager::instance()->getIcon(u"torrent-start"_qs, u"media-playback-start"_qs), tr("Resume torrents")
        , this, &CategoryFilterWidget::actionResumeTorrentsTriggered);
    menu->addAction(UIThemeManager::instance()->getIcon(u"torrent-stop"_qs, u"media-playback-pause"_qs), tr("Pause torrents")
        , this, &CategoryFilterWidget::actionPauseTorrentsTriggered);
    menu->addAction(UIThemeManager::instance()->getIcon(u"list-remove"_qs), tr("Remove torrents")
        , this, &CategoryFilterWidget::actionDeleteTorrentsTriggered);

    menu->popup(QCursor::pos());
}

void CategoryFilterWidget::callUpdateGeometry()
{
    if (!BitTorrent::Session::instance()->isSubcategoriesEnabled())
        setIndentation(0);
    else
        setIndentation(m_defaultIndentation);

    updateGeometry();
}

QSize CategoryFilterWidget::sizeHint() const
{
    // The sizeHint must depend on viewportSizeHint,
    // otherwise widget will not correctly adjust the
    // size when subcategories are used.
    const QSize viewportSize {viewportSizeHint()};
    return
    {
        viewportSize.width(),
        viewportSize.height() + static_cast<int>(0.5 * sizeHintForRow(0))
    };
}

QSize CategoryFilterWidget::minimumSizeHint() const
{
    QSize size = sizeHint();
    size.setWidth(6);
    return size;
}

void CategoryFilterWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);

    // Expand all parents if the parent(s) of the node are not expanded.
    QModelIndex p = parent;
    while (p.isValid())
    {
        if (!isExpanded(p))
            expand(p);
        p = model()->parent(p);
    }

    updateGeometry();
}

void CategoryFilterWidget::addCategory()
{
    TorrentCategoryDialog::createCategory(this);
}

void CategoryFilterWidget::addSubcategory()
{
    TorrentCategoryDialog::createCategory(this, currentCategory());
}

void CategoryFilterWidget::editCategory()
{
    TorrentCategoryDialog::editCategory(this, currentCategory());
}

void CategoryFilterWidget::removeCategory()
{
    const auto selectedRows = selectionModel()->selectedRows();
    if (!selectedRows.empty() && !CategoryFilterModel::isSpecialItem(selectedRows.first()))
    {
        BitTorrent::Session::instance()->removeCategory(
                    static_cast<CategoryFilterProxyModel *>(model())->categoryName(selectedRows.first()));
        updateGeometry();
    }
}

void CategoryFilterWidget::removeUnusedCategories()
{
    auto session = BitTorrent::Session::instance();
    for (const QString &category : asConst(session->categories()))
    {
        if (model()->data(static_cast<CategoryFilterProxyModel *>(model())->index(category), Qt::UserRole) == 0)
            session->removeCategory(category);
    }
    updateGeometry();
}
