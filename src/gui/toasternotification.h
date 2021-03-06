#ifndef TOASTERNOTIFICATION_H
#define TOASTERNOTIFICATION_H

#include "inotification.h"
class ToasterBox;

class ToasterNotification : public INotification
{
	public:
		ToasterNotification(wxWindow* parent);
		virtual ~ToasterNotification();
		virtual void Show(const wxBitmap& icon, const size_t pos, const UiEvents::NotficationData& data );
	protected:
		void SetPopupPosition( const size_t pos );
		ToasterBox* m_toasterbox;
		int m_width,m_height,m_x_offset,m_y_offset;
};

#endif // TOASTERNOTIFICATION_H

/**
	This file is part of SpringLobby,
	Copyright (C) 2007-2011

	SpringLobby is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as published by
	the Free Software Foundation.

	SpringLobby is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SpringLobby.  If not, see <http://www.gnu.org/licenses/>.
**/
