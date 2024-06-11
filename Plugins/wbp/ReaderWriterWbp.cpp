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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgDB/Registry"
#include "osgDB/ReaderWriter"

/** Implement the wbp format by wrapping any existing .webp reader/writer. */
class ReaderWriterWbp : public osgDB::ReaderWriter
{
public:
  ReaderWriterWbp()
  {
    // Detect the .webp loader, it will do all the heavy lifting
    webp_ = osgDB::Registry::instance()->getReaderWriterForExtension("webp");
    if (webp_.valid())
    {
      // Need to add support for .wbp in both plug-ins. Else .webp will reject filenames due to !acceptsExtension()
      // and the initial load will fail if we don't handle it, because .webp won't yet be in the map of plugins
      // that support that extension. So both need to handle.
      supportsExtension("wbp", "WebP image format");
      webp_->supportsExtension("wbp", "WebP image format");
    }
  }

  virtual const char *className() const override
  {
    return "Google WebP .wbp Image Reader/Writer";
  }

  virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const override
  {
    if (!webp_)
      return ReadResult::FILE_NOT_HANDLED;
    return webp_->readObject(file, options);
  }

  virtual ReadResult readObject(std::istream& fin, const Options* options) const override
  {
    if (!webp_)
      return ReadResult::FILE_NOT_HANDLED;
    return webp_->readObject(fin, options);
  }

  virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const override
  {
    if (!webp_)
      return ReadResult::FILE_NOT_HANDLED;
    return webp_->readImage(file, options);
  }

  virtual ReadResult readImage(std::istream& fin, const Options* options) const override
  {
    if (!webp_)
      return ReadResult::FILE_NOT_HANDLED;
    return webp_->readImage(fin, options);
  }

  virtual WriteResult writeObject(const osg::Object& object, const std::string& file, const osgDB::ReaderWriter::Options* options) const override
  {
    if (!webp_)
      return WriteResult::FILE_NOT_HANDLED;
    return webp_->writeObject(object, file, options);
  }

  virtual WriteResult writeObject(const osg::Object& object, std::ostream& fout, const Options* options) const override
  {
    if (!webp_)
      return WriteResult::FILE_NOT_HANDLED;
    return webp_->writeObject(object, fout, options);
  }

  virtual WriteResult writeImage(const osg::Image& img, const std::string& file, const osgDB::ReaderWriter::Options* options) const override
  {
    if (!webp_)
      return WriteResult::FILE_NOT_HANDLED;
    return webp_->writeImage(img, file, options);
  }

private:
  osg::ref_ptr<osgDB::ReaderWriter> webp_;
};

REGISTER_OSGPLUGIN(wbp, ReaderWriterWbp)
