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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <memory>
#include <QCoreApplication>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Math.h"
#include "simQt/SettingsModel.h"
#include "simQt/SettingsGroup.h"


namespace {

class ObserverCounter : public simQt::Settings::Observer
{
public:
  ObserverCounter() : counter_(0) {};
  virtual ~ObserverCounter() {};
  virtual void onSettingChange(const QString& name, const QVariant& value)
  {
    counter_++;
  }

  int counter() const { return counter_; }
private:
  int counter_;
};

class ObserverNameCheck : public simQt::Settings::Observer
{
public:
  ObserverNameCheck(const QString& expectedName, const QVariant& value) : expectedName_(expectedName), expectedValue_(value), result_(false) {};
  virtual ~ObserverNameCheck() {};
  virtual void onSettingChange(const QString& name, const QVariant& value)
  {
    result_ = ((name == expectedName_) && (value == expectedValue_));
  }

  bool gotExpected() const { return result_; }
  void clearExpected() { result_ = false; }
  void setExpectedNameAndValue(const QString& expectedName, const QVariant& value)
  {
    expectedName_ = expectedName;
    expectedValue_ = value;
    result_ = false;
  }

private:
  QString expectedName_;
  QVariant expectedValue_;
  bool result_;
};


bool areEqual(const simQt::Settings::MetaData& md1, const simQt::Settings::MetaData& md2)
{
  if ((md1.defaultValue() == md2.defaultValue()) &&
    (md1.level() == md2.level()) &&
    (md1.maxValue() == md2.maxValue()) &&
    (md1.minValue() == md2.minValue()) &&
    (md1.toolTip() == md2.toolTip()) &&
    (md1.type() == md2.type()) &&
    (md1.numDecimals() == md2.numDecimals()))
    return true;

  return false;
}


int testSingleLevel(simQt::SettingsPtr settings)
{
  int rv = 0;

  settings->clear();  // Start with a clean slate
  rv += SDK_ASSERT(settings->allNames().size() == 0);

  simQt::Settings::MetaData checkMeta;
  simQt::Settings::ObserverPtr goc(new ObserverCounter); // Global Observer Counter
  simQt::Settings::ObserverPtr oc(new ObserverCounter); // Observer Counter
  settings->addObserver(goc);

  // Test Failures
  rv += SDK_ASSERT(settings->contains("ShouldNotWork") == false);
  rv += SDK_ASSERT(settings->value("ShouldNotWork") == QVariant());
  rv += SDK_ASSERT(settings->setMetaData("ShouldNotWork", simQt::Settings::MetaData()) != 0);
  rv += SDK_ASSERT(settings->metaData("ShouldNotWork", checkMeta) != 0);
  rv += SDK_ASSERT(settings->removeObserver("ShouldNotWork", oc) != 0);
  rv += SDK_ASSERT(settings->removeObserver(oc) != 0);

  // Test Success with strings
  simQt::Settings::MetaData metaString(simQt::Settings::STRING, "Test", "Tool Tip", simQt::Settings::DEFAULT, "Min Value", "Max Value");
  rv += SDK_ASSERT(settings->value("WillWork", metaString, oc) == QVariant("Test").toString());
  rv += SDK_ASSERT(settings->contains("WillWork"));
  rv += SDK_ASSERT(settings->metaData("WillWork", checkMeta) == 0);
  rv += SDK_ASSERT(areEqual(metaString, checkMeta));
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 0); // check initial value
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 0);
  settings->setValue("WillWork", "NewValue");
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 1); // Should have incremented by one
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 1);
  rv += SDK_ASSERT(settings->value("WillWork") == QVariant("NewValue").toString());

  // Test Success with doubles
  simQt::Settings::MetaData metaDouble(simQt::Settings::DOUBLE, 1.0, "Tool Tip Double", simQt::Settings::PRIVATE, -10.0, 10.0);
  rv += SDK_ASSERT(settings->value("WillWorkDouble", metaDouble).toDouble() == 1.0);
  rv += SDK_ASSERT(settings->metaData("WillWorkDouble", checkMeta) == 0);
  rv += SDK_ASSERT(areEqual(metaDouble, checkMeta));
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 1); // check initial value
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 1);
  settings->setValue("WillWorkDouble", 2.0);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 2); // Should have incremented by one
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 1); // Should not change since callback for different entry
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 2.0);
  // Add in observer
  rv += SDK_ASSERT(settings->value("WillWorkDouble", oc).toDouble() == 2.0);
  settings->setValue("WillWorkDouble", 3.0);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 3); // Should have incremented by one
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 2); // Should now increment by one
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 3.0);

  // Test Skip observer
  settings->setValue("WillWorkDouble", 4.0, oc);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 4); // Should have incremented by one
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 2); // Should have been skipped
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 4.0);
  settings->setValue("WillWorkDouble", 5.0, goc);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 4); // Should have been skipped
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 3); //Should have incremented by one
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 5.0);

  // Test repeat values do not fire off callbacks
  settings->setValue("WillWorkDouble", 5.0);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 4); // Should not change
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 3); // Should not change
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 5.0);
  settings->setValue("WillWorkDouble", 5.0, oc);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 4); // Should not change
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 3); // Should not change
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 5.0);
  settings->setValue("WillWorkDouble", 5.0, goc);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 4); // Should not change
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 3); // Should not change
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 5.0);
  settings->setValue("WillWorkDouble", 5.0, metaDouble, goc);
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(goc.get())->counter() == 4); // Should not change
  rv += SDK_ASSERT(static_cast<ObserverCounter*>(oc.get())->counter() == 3); // Should not change
  rv += SDK_ASSERT(settings->value("WillWorkDouble").toDouble() == 5.0);

  // Make sure the default meta data does not override the initial setting
  settings->setValue("SetWithoutValue", "Yes", simQt::Settings::MetaData::makeString("No", "Yes or no"));
  rv += SDK_ASSERT(settings->value("SetWithoutValue").toString() == "Yes");

  // Get missing coverage
  settings->setValue("LastOne", 1, simQt::Settings::MetaData(simQt::Settings::INTEGER));
  rv += SDK_ASSERT(settings->value("LastOne").toInt() == 1);

  // Clean up
  rv += SDK_ASSERT(settings->removeObserver("WillWork", oc) == 0);
  rv += SDK_ASSERT(settings->removeObserver(goc) == 0);
  settings->clear();
  rv += SDK_ASSERT(settings->allNames().size() == 0);

  return rv;
}

// Test using two levels
int testMultipleLevels(simQt::SettingsGroupPtr settings)
{
  int rv = 0;

  settings->clear();  // Start with a clean slate
  rv += SDK_ASSERT(settings->allNames().size() == 0);

  simQt::Settings::MetaData checkMeta;
  simQt::Settings::ObserverPtr observer(new ObserverNameCheck("Level2", "New Value"));
  simQt::Settings::ObserverPtr secondObserver(new ObserverNameCheck("Level2", "New Value 2"));
  simQt::Settings::ObserverPtr globalObserver(new ObserverNameCheck("Level2", "New Value"));

  // Set a global observer
  settings->addObserver(globalObserver);

  // Set a value with observer
  simQt::Settings::MetaData metaString(simQt::Settings::STRING, "Test", "Tool Tip", simQt::Settings::DEFAULT, "Min Value", "Max Value");
  rv += SDK_ASSERT(settings->value("Level2", metaString, observer) == QVariant("Test").toString());
  settings->setValue("Level2", "New Value");
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(observer.get())->gotExpected());
  static_cast<ObserverNameCheck*>(observer.get())->clearExpected();
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(globalObserver.get())->gotExpected());
  static_cast<ObserverNameCheck*>(globalObserver.get())->setExpectedNameAndValue("Level2", "New Value 2");

  // Add second observer
  rv += SDK_ASSERT(settings->addObserver("Level2", secondObserver) == 0);

  // Set a value but skip over first observer
  settings->setValue("Level2", "New Value 2", observer);  // This time skip the observer
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(observer.get())->gotExpected() == false);  // Should be false since it got skipped
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(secondObserver.get())->gotExpected() == true);  // Should be true since it was not skipped
  rv += SDK_ASSERT(settings->value("Level2").toString() == "New Value 2");  // Value still should be set
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(globalObserver.get())->gotExpected());
  static_cast<ObserverNameCheck*>(globalObserver.get())->setExpectedNameAndValue("Level2", "New Value");

  // Set a value with observer
  settings->setValue("Level2", "New Value");
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(observer.get())->gotExpected());
  static_cast<ObserverNameCheck*>(observer.get())->clearExpected();
  static_cast<ObserverNameCheck*>(secondObserver.get())->clearExpected();
  rv += SDK_ASSERT(settings->value("Level2").toString() == "New Value");  // Value still should be set
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(globalObserver.get())->gotExpected());
  static_cast<ObserverNameCheck*>(globalObserver.get())->clearExpected();

  // Set a value after removing the observers
  rv += SDK_ASSERT(settings->removeObserver("Level2", observer) == 0);
  rv += SDK_ASSERT(settings->removeObserver("Level2", secondObserver) == 0);
  rv += SDK_ASSERT(settings->removeObserver(globalObserver) == 0);
  settings->setValue("Level2", "New Value 2");
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(observer.get())->gotExpected() == false);  // Should be false since no callback
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(secondObserver.get())->gotExpected() == false);  // Should be false since no callback
  rv += SDK_ASSERT(static_cast<ObserverNameCheck*>(globalObserver.get())->gotExpected() == false);  // Should be false since no callback
  rv += SDK_ASSERT(settings->value("Level2").toString() == "New Value 2");  // Value still should be set

  return rv;
}

int testSettingsWithoutMetaData()
{
  QSettings settings;
  settings.clear();
  // Set a single value without meta data (as if from a .ini file or registry)
  settings.setValue("Setting1", "1");
  simQt::SettingsModel model(NULL, settings);

  int rv = 0;
  rv += SDK_ASSERT(model.allNames().size() == 1);
  // Should only contain "Setting1"
  rv += SDK_ASSERT(model.rowCount() == 1);
  rv += SDK_ASSERT(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Setting1");
  rv += SDK_ASSERT(model.data(model.index(0, 1), Qt::DisplayRole).toString() == "1");

  // setValue without meta data or a seeding value()
  model.setValue("Setting2", "2");
  // Should only contain "Setting1" and "Setting2"
  rv += SDK_ASSERT(model.rowCount() == 2);
  rv += SDK_ASSERT(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Setting1");
  rv += SDK_ASSERT(model.data(model.index(0, 1), Qt::DisplayRole).toString() == "1");
  rv += SDK_ASSERT(model.data(model.index(1, 0), Qt::DisplayRole).toString() == "Setting2");
  rv += SDK_ASSERT(model.data(model.index(1, 1), Qt::DisplayRole).toString() == "2");
  // Change Setting2 manually and make sure model reflects it
  model.setValue("Setting2", "02");
  rv += SDK_ASSERT(model.rowCount() == 2);
  rv += SDK_ASSERT(model.data(model.index(1, 1), Qt::DisplayRole).toString() == "02");

  return rv;
}

int testPersistentMetaData()
{
  int rv = 0;

  // Force a registration of the simQt::Settings::MetaData, even though it's also done elsewhere,
  // so that this test is more standalone.  Without this, a SettingsModel is required to instantiate.
  qRegisterMetaTypeStreamOperators<simQt::Settings::MetaData>("simQt::Settings::MetaData");


  // Create a few sample meta data items
  QMap<int, QString> enumValues;
  enumValues.insert(1, "one");
  enumValues.insert(3, "three");
  simQt::Settings::MetaData stringMD = simQt::Settings::MetaData::makeString("str", "tt1", simQt::Settings::ADVANCED);
  simQt::Settings::MetaData enumMD = simQt::Settings::MetaData::makeEnumeration(1, enumValues, "tt2", simQt::Settings::DEFAULT);
  simQt::Settings::MetaData doubleMD = simQt::Settings::MetaData::makeDouble(3.0, "tt3", simQt::Settings::PRIVATE, 0.1, 3.3, 1);
  simQt::Settings::MetaData fileMD = simQt::Settings::MetaData::makeFilename("foo.txt", "tt4", simQt::Settings::DEFAULT, "All files (*)");

  QSettings settings;
  settings.setValue("stringMD", QVariant::fromValue(stringMD));
  settings.setValue("enumMD", QVariant::fromValue(enumMD));
  settings.setValue("doubleMD", QVariant::fromValue(doubleMD));
  settings.setValue("fileMD", QVariant::fromValue(fileMD));
  simQt::Settings::MetaData fromSettings;

  // Pull the data back out for string
  QVariant qvString = settings.value("stringMD", QVariant());
  rv += SDK_ASSERT(qvString.isValid());
  rv += SDK_ASSERT(qvString.canConvert<simQt::Settings::MetaData>());
  fromSettings = qvString.value<simQt::Settings::MetaData>();
  rv += SDK_ASSERT(fromSettings.defaultValue().toString() == "str");
  rv += SDK_ASSERT(fromSettings.toolTip() == "tt1");
  rv += SDK_ASSERT(fromSettings.level() == simQt::Settings::ADVANCED);
  rv += SDK_ASSERT(fromSettings.type() == simQt::Settings::STRING);

  // Pull the data back out for enum
  QVariant qvEnum = settings.value("enumMD", QVariant());
  rv += SDK_ASSERT(qvEnum.isValid());
  rv += SDK_ASSERT(qvEnum.canConvert<simQt::Settings::MetaData>());
  fromSettings = qvEnum.value<simQt::Settings::MetaData>();
  rv += SDK_ASSERT(fromSettings.defaultValue().toInt() == 1);
  rv += SDK_ASSERT(fromSettings.toolTip() == "tt2");
  rv += SDK_ASSERT(fromSettings.level() == simQt::Settings::DEFAULT);
  rv += SDK_ASSERT(fromSettings.type() == simQt::Settings::ENUMERATION);
  rv += SDK_ASSERT(fromSettings.enumValues().size() == 2);
  rv += SDK_ASSERT(fromSettings.enumValues()[1] == "one");
  rv += SDK_ASSERT(fromSettings.enumValues()[3] == "three");

  // Pull out data for double
  QVariant qvDouble = settings.value("doubleMD", QVariant());
  rv += SDK_ASSERT(qvDouble.isValid());
  rv += SDK_ASSERT(qvDouble.canConvert<simQt::Settings::MetaData>());
  fromSettings = qvDouble.value<simQt::Settings::MetaData>();
  rv += SDK_ASSERT(simCore::areEqual(fromSettings.defaultValue().toDouble(), 3.0));
  rv += SDK_ASSERT(simCore::areEqual(fromSettings.minValue().toDouble(), 0.1));
  rv += SDK_ASSERT(simCore::areEqual(fromSettings.maxValue().toDouble(), 3.3));
  rv += SDK_ASSERT(fromSettings.toolTip() == "tt3");
  rv += SDK_ASSERT(fromSettings.numDecimals() == 1);
  rv += SDK_ASSERT(fromSettings.level() == simQt::Settings::PRIVATE);
  rv += SDK_ASSERT(fromSettings.type() == simQt::Settings::DOUBLE);

  // Pull out filename data
  QVariant qvFile = settings.value("fileMD", QVariant());
  rv += SDK_ASSERT(qvFile.isValid());
  rv += SDK_ASSERT(qvFile.canConvert<simQt::Settings::MetaData>());
  fromSettings = qvFile.value<simQt::Settings::MetaData>();
  rv += SDK_ASSERT(fromSettings.defaultValue().toString() == "foo.txt");
  rv += SDK_ASSERT(fromSettings.toolTip() == "tt4");
  rv += SDK_ASSERT(fromSettings.level() == simQt::Settings::DEFAULT);
  rv += SDK_ASSERT(fromSettings.type() == simQt::Settings::FILENAME);
  rv += SDK_ASSERT(fromSettings.filenameFilter() == "All files (*)");

  return rv;
}

int testResetDefaults(simQt::SettingsGroupPtr settings)
{
  int rv = 0;
  settings->clear();

  // Set values and defaults
  settings->setValue("DefaultInt", 8, simQt::Settings::MetaData::makeInteger(3));
  settings->setValue("DefaultDouble", 8.0, simQt::Settings::MetaData::makeDouble(3.0));
  settings->setValue("DefaultString", "temp", simQt::Settings::MetaData::makeString("default"));

  // Reset DefaultInt only
  settings->resetDefaults("DefaultInt");
  rv += SDK_ASSERT(settings->value("DefaultInt").toInt() == 3);
  rv += SDK_ASSERT(settings->value("DefaultDouble").toDouble() == 8.0);
  rv += SDK_ASSERT(settings->value("DefaultString").toString() == "temp");

  // Reset all
  settings->resetDefaults();
  rv += SDK_ASSERT(settings->value("DefaultInt").toInt() == 3);
  rv += SDK_ASSERT(settings->value("DefaultDouble").toDouble() == 3.0);
  rv += SDK_ASSERT(settings->value("DefaultString").toString() == "default");

  // Clean up
  settings->clear();
  rv += SDK_ASSERT(settings->allNames().size() == 0);

  return rv;
}

int testMemento(simQt::Settings& settings)
{
  settings.clear();

  // Set values with metadata
  settings.setValue("DefaultInt", 8, simQt::Settings::MetaData::makeInteger(3));
  settings.setValue("DefaultDouble", 8.0, simQt::Settings::MetaData::makeDouble(3.0));
  settings.setValue("DefaultString", "temp", simQt::Settings::MetaData::makeString("default"));
  settings.setValue("AnotherInt", 7, simQt::Settings::MetaData::makeInteger(11));

  int rv = 0;
  // Test values
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 8);
  rv += SDK_ASSERT(settings.value("DefaultDouble").toDouble() == 8.0);
  rv += SDK_ASSERT(settings.value("DefaultString").toString() == "temp");
  rv += SDK_ASSERT(settings.value("AnotherInt").toInt() == 7);

  // Save a memento
  std::unique_ptr<simQt::Settings::Memento> before(settings.createMemento());
  rv += SDK_ASSERT(before != NULL);

  // Change values in unexpected ways
  settings.setValue("DefaultInt", 4.5);
  settings.setValue("DefaultDouble", "foo");
  settings.setValue("DefaultString", 1);

  // Test that the value changes took effect
  rv += SDK_ASSERT(settings.value("DefaultInt").toDouble() == 4.5);
  rv += SDK_ASSERT(settings.value("DefaultDouble").toString() == "foo");
  rv += SDK_ASSERT(settings.value("DefaultString").toInt() == 1);
  rv += SDK_ASSERT(settings.value("AnotherInt").toInt() == 7);

  std::unique_ptr<simQt::Settings::Memento> after(settings.createMemento());
  rv += SDK_ASSERT(after != NULL);

  // Test values after restoring the old memento
  rv += SDK_ASSERT(before->restore(settings) == 0);
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 8);
  rv += SDK_ASSERT(settings.value("DefaultDouble").toDouble() == 8.0);
  rv += SDK_ASSERT(settings.value("DefaultString").toString() == "temp");
  rv += SDK_ASSERT(settings.value("AnotherInt").toInt() == 7);

  // Now go back to the after state
  rv += SDK_ASSERT(after->restore(settings) == 0);
  rv += SDK_ASSERT(settings.value("DefaultInt").toDouble() == 4.5);
  rv += SDK_ASSERT(settings.value("DefaultDouble").toString() == "foo");
  rv += SDK_ASSERT(settings.value("DefaultString").toInt() == 1);
  rv += SDK_ASSERT(settings.value("AnotherInt").toInt() == 7);

  // Reuse the before memento to make sure it's not a one-time use
  rv += SDK_ASSERT(before->restore(settings) == 0);
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 8);
  rv += SDK_ASSERT(settings.value("DefaultDouble").toDouble() == 8.0);
  rv += SDK_ASSERT(settings.value("DefaultString").toString() == "temp");
  rv += SDK_ASSERT(settings.value("AnotherInt").toInt() == 7);

  return rv;
}

int testMementoSubgroup(simQt::Settings& settings)
{
  settings.clear();

  // Create a sub-grouping and make sure memento there doesn't impact memento in top level
  simQt::SettingsGroupPtr alevel(new simQt::SettingsGroup(&settings, "ALevel"));
  alevel->setValue("value", 1);
  settings.setValue("DefaultInt", 11);
  int rv = 0;
  rv += SDK_ASSERT(alevel->value("value").toInt() == 1);
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 11);

  std::unique_ptr<simQt::Settings::Memento> before2(alevel->createMemento());
  alevel->setValue("value", 2);
  rv += SDK_ASSERT(alevel->value("value").toInt() == 2);
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 11);
  settings.setValue("DefaultInt", 12);
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 12);

  std::unique_ptr<simQt::Settings::Memento> after2(alevel->createMemento());
  rv += SDK_ASSERT(before2->restore(*alevel) == 0);
  rv += SDK_ASSERT(alevel->value("value").toInt() == 1);
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 12); // shouldn't change

  rv += SDK_ASSERT(after2->restore(*alevel) == 0);
  rv += SDK_ASSERT(alevel->value("value").toInt() == 2);
  rv += SDK_ASSERT(settings.value("DefaultInt").toInt() == 12); // shouldn't change

  return rv;
}

}

int SettingsTest(int argc, char* argv[])
{
  int rv = 0;

  QCoreApplication::setOrganizationName("Naval Research Laboratory");
  QCoreApplication::setApplicationName("simQt Settings Test Application");

  // Test meta data first since other tests implicitly use it
  rv += testPersistentMetaData();

  QSettings qSettings;
  simQt::SettingsPtr settings(new simQt::SettingsModel(NULL, qSettings));
  rv += testSingleLevel(settings);

  simQt::SettingsGroupPtr group(new simQt::SettingsGroup(settings.get(), ""));
  rv += testSingleLevel(group);

  simQt::SettingsGroupPtr group2(new simQt::SettingsGroup(settings.get(), "ALevel"));
  rv += testMultipleLevels(group2);

  simQt::SettingsGroupPtr group3(new simQt::SettingsGroup(settings.get(), "TestDefaults"));
  rv += testResetDefaults(group3);

  rv += testSettingsWithoutMetaData();

  rv += testMemento(*settings);
  rv += testMemento(*group2);
  rv += testMementoSubgroup(*settings);

  return rv;
}
