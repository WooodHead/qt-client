/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "changeWoQty.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

changeWoQty::changeWoQty(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_change, SIGNAL(clicked()), this, SLOT(sChangeQty()));
  connect(_newQtyOrdered, SIGNAL(textChanged(const QString&)), this, SLOT(sQtyChanged(const QString&)));

  _captive = FALSE;

  _wo->setType(cWoOpen | cWoExploded);
  _newQtyOrdered->setValidator(omfgThis->qtyVal());
  _newQtyReceived->setPrecision(omfgThis->qtyVal());
  _newQtyBalance->setPrecision(omfgThis->qtyVal());
  _currentQtyOrdered->setPrecision(omfgThis->qtyVal());
  _currentQtyReceived->setPrecision(omfgThis->qtyVal());
  _currentQtyBalance->setPrecision(omfgThis->qtyVal());
  _cmnttype->setType(XComboBox::AllCommentTypes);
  _commentGroup->setEnabled(_postComment->isChecked());
}

changeWoQty::~changeWoQty()
{
  // no need to delete child widgets, Qt does it all for us
}

void changeWoQty::languageChange()
{
  retranslateUi(this);
}

enum SetResponse changeWoQty::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
    _newQtyOrdered->setFocus();
  }

  param = pParams.value("newQty", &valid);
  if (valid)
    _newQtyOrdered->setDouble(param.toDouble());

  return NoError;
}

void changeWoQty::sChangeQty()
{
  if (_wo->status() == 'R')
  {
    QMessageBox::warning( this, tr("Cannot Reschedule Released W/O"),
                          tr( "<p>The selected Work Order has been Released. "
			  "Rescheduling it." ) );
    return;
  }

  double    newQty = _newQtyOrdered->toDouble();

  if(newQty == 0 && QMessageBox::question(this, tr("Zero Qty. Value"), tr("The current value specified is 0. Are you sure you want to continue?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  q.prepare( "SELECT validateOrderQty(wo_itemsite_id, :qty, TRUE) AS qty "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  q.bindValue(":wo_id", _wo->id());
  q.bindValue(":qty", _newQtyOrdered->toDouble());
  q.exec();
  if (q.first())
  {
    if (q.value("qty").toDouble() != newQty)
    {
      if ( QMessageBox::warning( this, tr("Invalid Order Qty"),
                                 tr("<p>The new Order Quantity that you have "
				 "entered does not meet the Order Parameters "
				 "set for the parent Item Site for this Work "
				 "Order.  In order to meet the Item Site "
				 "Order Parameters the new Order Quantity "
				 "must be increased to %1. Do you want to "
				 "change the Order Quantity for this Work "
				 "Order to %2?" )
                                 .arg(formatQty(q.value("qty").toDouble()))
                                 .arg(formatQty(q.value("qty").toDouble())),
                                 tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 1 )
        return;
      else
        newQty = q.value("qty").toDouble();
    }

    q.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE);");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":qty", newQty);
    q.exec();

    if (_postComment->isChecked())
    {
      q.prepare("SELECT postComment(:cmnttype_id, 'W', :wo_id, :comment) AS result");
      q.bindValue(":cmnttype_id", _cmnttype->id());
      q.bindValue(":wo_id", _wo->id());
      q.bindValue(":comment", _comment->toPlainText());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("postComment", result),
                      __FILE__, __LINE__);
          return;
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  accept();
}

void changeWoQty::sQtyChanged(const QString &pNewQty)
{
  double qtyBalance = (pNewQty.toDouble() - _newQtyReceived->toDouble());
  if (qtyBalance < 0)
    qtyBalance = 0;

  _newQtyBalance->setDouble(qtyBalance);
}
