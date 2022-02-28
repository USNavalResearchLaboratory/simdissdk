/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simData/DataStore.h"
#include "simData/DataTable.h"

namespace simData
{
//----------------------------------------------------------------------------
DataStore::Transaction::Transaction()
{
}

DataStore::Transaction::Transaction(TransactionImpl *actual)
: transaction_(actual)
{
}

DataStore::Transaction::Transaction(const Transaction &transaction)
: transaction_(transaction.transaction_)
{
}

DataStore::Transaction::~Transaction()
{
}

DataStore::Transaction& DataStore::Transaction::operator=(const Transaction &rhs)
{
  transaction_ = rhs.transaction_;
  return *this;
}

void DataStore::Transaction::commit()
{
  assert(transaction_.get() != nullptr);
  transaction_->commit();
}

//----------------------------------------------------------------------------
DataStore::InternalsMemento::~InternalsMemento()
{
}

DataStore::~DataStore()
{
}

} // namespace simData

