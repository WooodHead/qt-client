/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SEARCHFORCONTACT_H
#define SEARCHFORCONCACT_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_searchForContact.h"

class searchForContact : public XWidget, public Ui::searchForContact
{
    Q_OBJECT

public:
    searchForContact(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~searchForContact();

public slots:
    virtual void	sEdit();
    virtual void	sFillList();
    virtual void	sPopulateMenu(QMenu * pMenu);
    virtual void	sView();
	virtual void	sDelete();

protected slots:
    virtual void languageChange();

protected:
    bool				_editpriv;
    bool				_viewpriv;


};

#endif // SEARCHFORCONTACT_H