/*
 * Copyright (c) 2022 Belledonne Communications SARL.
 *
 * This file is part of linphone-desktop
 * (see https://www.linphone.org).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SORT_FILTER_ABSTRACT_PROXY_MODEL_H_
#define SORT_FILTER_ABSTRACT_PROXY_MODEL_H_

#include "SortFilterProxyModel.hpp"

template <class T>
class SortFilterAbstractProxyModel : public SortFilterProxyModel {
public:
	SortFilterAbstractProxyModel(T * model, QObject * parent = nullptr) : SortFilterProxyModel(parent){
		update(model);
	}
	
	void update(T* model){
		setSourceModel(model);
		sort(0, Qt::DescendingOrder);
	}
	
	template <class X>
	void add(QSharedPointer<X> x){
		qobject_cast<T*>(sourceModel())->add(x);
	}
};


#endif
