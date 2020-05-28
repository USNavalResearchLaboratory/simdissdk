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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include "simQt/SettingsGroup.h"

namespace simQt {

namespace {

/// Class to convert a full path callback into a local path callback
class LocalWrappedObserver : public Settings::Observer
{
public:
  LocalWrappedObserver(const QString& prefix, const Settings::ObserverPtr& myObserver)
    : prefix_(prefix), myObserver_(myObserver)
  {
  }
  // Strip the prefix off the name and redo the callback
  virtual void onSettingChange(const QString& name, const QVariant& value)
  {
    if (name.startsWith(prefix_))
    {
      QString newName = name;
      newName.replace(0, prefix_.length(), "");
      myObserver_->onSettingChange(newName, value);
    }
    else
    {
      // The caller passed in a global name, so need to return a global name
      myObserver_->onSettingChange("/" + name, value);
    }
  }
  Settings::ObserverPtr unwrappedObserver() const
  {
    return myObserver_;
  }
private:
  QString prefix_;  ///< The prefix to strip off the name
  Settings::ObserverPtr myObserver_;  ///< The original callback
};

/// Class to convert a full path callback into a group path callback
class GlobalWrappedObserver : public Settings::Observer
{
public:
  GlobalWrappedObserver(const QString& prefix, const Settings::ObserverPtr& myObserver)
    : prefix_(prefix), myObserver_(myObserver)
  {
  }

  // If the prefix matches, strip the prefix off the name and redo the callback
  virtual void onSettingChange(const QString& name, const QVariant& value)
  {
    if (name.startsWith(prefix_))
    {
      QString newName = name;
      newName.replace(0, prefix_.length(), "");
      myObserver_->onSettingChange(newName, value);
    }
  }
  Settings::ObserverPtr unwrappedObserver() const
  {
    return myObserver_;
  }
private:
  QString prefix_;  ///< The prefix to strip off the name
  Settings::ObserverPtr myObserver_;  ///< The original callback
};

}


SettingsGroup::SettingsGroup(Settings* settings, const QString& path)
  : Settings(),
    settings_(settings),
    path_(path)
{
  // If necessary add / to a non-empty string
  if (!path.isEmpty() && !path_.endsWith("/"))
    path_ += "/";
}

SettingsGroup::~SettingsGroup()
{
  // Cleanup local
  for (QMap<QString, QList<ObserverPtr> >::iterator mapIter = localObservers_.begin(); mapIter != localObservers_.end(); ++mapIter)
  {
    for (QList<ObserverPtr>::iterator it = mapIter->begin(); it != mapIter->end(); ++it)
    {
      settings_->removeObserver(getFullPath_(mapIter.key()), *it);
    }
  }

  // Cleanup global
  for (QList<ObserverPtr>::iterator it = globalObservers_.begin(); it != globalObservers_.end(); ++it)
  {
    settings_->removeObserver(*it);
  }
}

QString SettingsGroup::getFullPath_(const QString& name) const
{
  // leading slash means given name is already a full path, so strip preceding slash and return
  if (name.startsWith('/'))
    return name.right(name.length() - 1);
  return path_ + name;
}

int SettingsGroup::getObserver_(const QString& name, ObserverPtr unwrapped, ObserverPtr& wrapped) const
{
  QMap<QString, QList<ObserverPtr> >::const_iterator mapIter = localObservers_.find(name);
  if (mapIter == localObservers_.end())
    return 1;

  for (QList<ObserverPtr>::const_iterator it = mapIter->begin(); it != mapIter->end(); ++it)
  {
    if (static_cast<LocalWrappedObserver*>(it->get())->unwrappedObserver() == unwrapped)
    {
      wrapped = *it;
      return 0;
    }
  }
  return 1;
}

void SettingsGroup::clear()
{
  if (settings_)
    settings_->clear();
}

void SettingsGroup::resetDefaults()
{
  if (settings_)
    settings_->resetDefaults();
}

void SettingsGroup::resetDefaults(const QString& name)
{
  settings_->resetDefaults(getFullPath_(name));
}

void SettingsGroup::setValue(const QString& name, const QVariant& value)
{
  if (settings_)
    settings_->setValue(getFullPath_(name), value);
}

void SettingsGroup::setValue(const QString& name, const QVariant& value, const MetaData& metaData, ObserverPtr observer)
{
  if (settings_)
  {
     settings_->setValue(getFullPath_(name), value, metaData);
     addObserver(name, observer);
  }
}

void SettingsGroup::setValue(const QString& name, const QVariant& value, ObserverPtr skipThisObserver)
{
  if (settings_)
  {
    if (path_.isEmpty())
      settings_->setValue(name, value, skipThisObserver);
    else
    {
      if (skipThisObserver == NULL)
        settings_->setValue(getFullPath_(name), value);
      else
      {
        ObserverPtr wrappedObserver;
        int rv = getObserver_(name, skipThisObserver, wrappedObserver);
        assert(rv == 0);
        if (rv == 0)
          settings_->setValue(getFullPath_(name), value, wrappedObserver);
      }
    }
  }
}

QVariant SettingsGroup::value(const QString& name) const
{
  if (settings_)
    return settings_->value(getFullPath_(name));

  return QVariant();
}

QVariant SettingsGroup::value(const QString& name, const MetaData& metaData, ObserverPtr observer)
{
  if (settings_)
  {
    QVariant rv = settings_->value(getFullPath_(name), metaData);
    addObserver(name, observer);
    return rv;
  }

  return QVariant();
}

QVariant SettingsGroup::value(const QString& name, ObserverPtr observer)
{
  if (settings_)
  {
    QVariant rv = settings_->value(getFullPath_(name));
    addObserver(name, observer);
    return rv;
  }

  return QVariant();
}

bool SettingsGroup::contains(const QString& name) const
{
  if (settings_)
    return settings_->contains(getFullPath_(name));

  return false;
}

void SettingsGroup::saveWidget(QWidget* widget)
{
  if (settings_)
    settings_->saveWidget(widget);
}

void SettingsGroup::loadWidget(QWidget* widget)
{
  if (settings_)
    settings_->loadWidget(widget);
}

QStringList SettingsGroup::allNames() const
{
  if (settings_)
    return settings_->allNames();

  return QStringList();
}

int SettingsGroup::setMetaData(const QString& name, const MetaData& metaData)
{
  if (settings_)
    return settings_->setMetaData(getFullPath_(name), metaData);

  return 1;
}

int SettingsGroup::metaData(const QString& name, MetaData& metaData) const
{
  if (settings_)
    return settings_->metaData(getFullPath_(name), metaData);

  return 1;
}

int SettingsGroup::addObserver(const QString& name, ObserverPtr observer)
{
  if (settings_)
  {
    if (observer != NULL)
    {
      if (!path_.isEmpty())
      {
        ObserverPtr newObserver(new LocalWrappedObserver(path_, observer));
        localObservers_[name].push_back(newObserver);
        return settings_->addObserver(getFullPath_(name), newObserver);
      }

      return settings_->addObserver(name, observer);
    }
    return 0;
  }

  return 1;
}

int SettingsGroup::removeObserver(const QString& name, ObserverPtr observer)
{
  if (settings_)
  {
    if (observer != NULL)
    {
      if (!path_.isEmpty())
      {
        ObserverPtr wrappedObserver;
        int rv = getObserver_(name, observer, wrappedObserver);
        assert(rv == 0);
        if (rv == 0)
        {
           rv = settings_->removeObserver(getFullPath_(name), wrappedObserver);
           for (QList<ObserverPtr>::iterator it = localObservers_[name].begin(); it != localObservers_[name].end(); ++it)
           {
             if (*it == wrappedObserver)
             {
               localObservers_[name].erase(it);
               break;
             }
           }
        }
        return rv;
      }

      return settings_->removeObserver(name, observer);
    }
  }

  return 1;
}

void SettingsGroup::addObserver(ObserverPtr observer)
{
  if (settings_)
  {
    if (!path_.isEmpty())
    {
      ObserverPtr newObserver(new GlobalWrappedObserver(path_, observer));
      globalObservers_.push_back(newObserver);
      settings_->addObserver(newObserver);
    }
    else
      settings_->addObserver(observer);
  }
}

int SettingsGroup::removeObserver(ObserverPtr observer)
{
  if (settings_)
  {
    if (!path_.isEmpty())
    {
      for (QList<ObserverPtr>::iterator it = globalObservers_.begin(); it != globalObservers_.end(); ++it)
      {
        if (static_cast<GlobalWrappedObserver*>(it->get())->unwrappedObserver() == observer)
        {
          int rv = settings_->removeObserver(*it);
          globalObservers_.erase(it);
          return rv;
        }
      }

      return 1;
    }

    return settings_->removeObserver(observer);
  }
  return 1;
}

QString SettingsGroup::fileName() const
{
  if (settings_)
    return settings_->fileName();
  return QString();
}

/** Helper class to save and restore state of a Settings Group */
class SettingsGroupMemento : public Settings::Memento
{
public:
  SettingsGroupMemento(simQt::Settings* settings, const QString& path)
  {
    if (settings)
    {
      // Get a path that doesn't start with '/'
      QString pathNoStartingSlash = path;
      if (path.startsWith('/'))
        pathNoStartingSlash = path.right(path.length() - 1);

      // Loop through all values and save them
      const QStringList allNames = settings->allNames();
      for (auto it = allNames.begin(); it != allNames.end(); ++it)
      {
        const QString& variable = *it;
        if (variable.startsWith(pathNoStartingSlash))
          values_[QString("/%1").arg(variable)] = settings->value(variable);
      }
    }
  }

  virtual int restore(Settings& settings) const
  {
    for (auto iter = values_.begin(); iter != values_.end(); ++iter)
      settings.setValue(iter->first, iter->second);
    return 0;
  }

private:
  std::map<QString, QVariant> values_;
};

Settings::Memento* SettingsGroup::createMemento() const
{
  // We cannot just save out the settings and restore because the path will be wrong on restore
  return new SettingsGroupMemento(settings_, path_);
}

}

