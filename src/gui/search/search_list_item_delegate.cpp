/**
 * Taiga
 * Copyright (C) 2010-2024, Eren Okka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "search_list_item_delegate.hpp"

#include <QImageReader>
#include <QListView>
#include <QPainter>
#include <QPainterPath>

#include "gui/search/search_list_model.hpp"
#include "gui/utils/painter_state_saver.hpp"
#include "gui/utils/theme.hpp"

namespace gui {

SearchListItemDelegate::SearchListItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {
  QImageReader reader("./data/poster.jpg");
  const QImage image = reader.read();
  if (!image.isNull()) {
    m_pixmap = QPixmap::fromImage(image);
  }
}

void SearchListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  const PainterStateSaver painterStateSaver(painter);

  const auto model = static_cast<const SearchListModel*>(index.model());
  const auto item = model->getAnime(index);

  QStyleOptionViewItem opt = option;
  QRect rect = opt.rect;

  QPainterPath path;
  path.addRoundedRect(rect, 4, 4);
  painter->setClipPath(path);

  // Background
  if (option.state & QStyle::State_Selected) {
    painter->fillRect(rect, opt.palette.highlight());
  } else if (theme.isDark()) {
    painter->fillRect(rect, opt.palette.mid());
  } else {
    painter->fillRect(rect, opt.palette.alternateBase());
  }

  // Poster
  {
    QRect posterRect = rect;
    posterRect.setWidth(140);

    if (theme.isDark()) {
      painter->fillRect(posterRect, opt.palette.dark());
    } else {
      painter->fillRect(posterRect, opt.palette.mid());
    }
    painter->drawPixmap(posterRect, m_pixmap);

    rect.adjust(140, 0, 0, 0);
  }

  auto font = painter->font();

  // Title
  {
    QRect titleRect = rect;
    titleRect.setHeight(24);

    painter->fillRect(titleRect, opt.palette.dark());
    titleRect.adjust(8, 0, -8, 0);

    auto titleFont = painter->font();
    titleFont.setWeight(QFont::Weight::DemiBold);
    painter->setFont(titleFont);

    const QString title = index.data(Qt::DisplayRole).toString();
    const QFontMetrics metrics(painter->font());
    const QString elidedTitle = metrics.elidedText(title, Qt::ElideRight, titleRect.width());

    painter->drawText(titleRect, Qt::AlignVCenter | Qt::TextSingleLine, elidedTitle);

    rect.adjust(8, 24 + 8, -8, -8);
  }

  // Summary
  {
    const QString summary = u"TV · %1 episodes · %2"_qs.arg(item->episode_count).arg(item->score);
    const QFontMetrics metrics(painter->font());
    QRect summaryRect = rect;
    summaryRect.setHeight(metrics.height());
    painter->setFont(font);
    painter->drawText(summaryRect, Qt::AlignVCenter | Qt::TextSingleLine, summary);
    rect.adjust(0, summaryRect.height() + 8, 0, 0);
  }

  auto detailsFont = painter->font();

  // Details
  {
    const QString titles =
        "Aired:\n"
        "Genres:\n"
        "Studios:";
    const QString values =
        "Jan 7, 2024 to Mar 31, 2024 (Airing)\n"
        "Action, Adventure, Fantasy\n"
        "A-1 Pictures";

    detailsFont.setWeight(QFont::Weight::DemiBold);
    painter->setFont(detailsFont);

    const QFontMetrics metrics(painter->font());

    QRect titlesRect = rect;
    titlesRect.setHeight(metrics.height() * 3);
    titlesRect.setWidth(metrics.boundingRect("Studios:").width());

    painter->drawText(titlesRect, 0, titles);

    detailsFont.setWeight(QFont::Weight::Normal);
    painter->setFont(detailsFont);

    QRect valuesRect = rect;
    valuesRect.setHeight(metrics.height() * 3);
    valuesRect.adjust(titlesRect.width() + 8, 0, 0, 0);

    painter->drawText(valuesRect, 0, values);

    rect.adjust(0, titlesRect.height() + 8, 0, 0);
  }

  // Synopsis
  {
    const QString synopsis = QString::fromStdString(item->synopsis);

    painter->setPen(opt.palette.placeholderText().color());

    detailsFont.setPointSize(8);
    painter->setFont(detailsFont);
    const QFontMetrics metrics(painter->font());

    QRect synopsisRect = rect;
    synopsisRect.setHeight(qMin(synopsisRect.height(), metrics.height() * 5));

    painter->drawText(synopsisRect, Qt::TextWordWrap, synopsis);
  }
}

QSize SearchListItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
  if (index.isValid()) return itemSize();
  return QStyledItemDelegate::sizeHint(option, index);
}

void SearchListItemDelegate::initStyleOption(QStyleOptionViewItem* option,
                                             const QModelIndex& index) const {
  QStyledItemDelegate::initStyleOption(option, index);

  option->features &= ~QStyleOptionViewItem::ViewItemFeature::HasDisplay;
  option->features &= ~QStyleOptionViewItem::ViewItemFeature::HasDecoration;
}

QSize SearchListItemDelegate::itemSize() const {
  const auto parent = reinterpret_cast<QListView*>(this->parent());
  const auto rect = parent->geometry();

  constexpr int maxWidth = 360;
  int columns = 1;
  if (rect.width() > maxWidth * 2) columns = 2;
  if (rect.width() > maxWidth * 3) columns = 3;
  if (rect.width() > maxWidth * 4) columns = 4;

  constexpr int spacing = 18;
  const int width = (rect.width() - (spacing * (columns + 2))) / columns;
  constexpr int height = 210;

  return QSize(width, height);
}

}  // namespace gui
