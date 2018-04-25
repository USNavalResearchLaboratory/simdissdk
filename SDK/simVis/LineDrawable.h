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
#ifndef SIMVIS_LINEDRAWABLE_H
#define SIMVIS_LINEDRAWABLE_H 1

#include "osgEarth/Version"

#if OSGEARTH_VERSION_GREATER_OR_EQUAL(2,10,0)

#include "osgEarth/LineDrawable"

#else

#include "osgEarth/Common"
#include "osg/Geometry"
#include "osg/Geode"
#include "simCore/Common/Common.h"

/**
 * @file simVis/LineDrawable.h
 * This is a copy of osgEarth::LineDrawable used to bridge the gap between osgEarth 2.9
 * and the next stable release.  This file will be removed after the September 2018
 * release of the SIMDIS SDK (version 1.9).
 *
 * TODO: Remove this and associated files after the September 2018 1.9 SIMDIS SDK release.
 */

namespace osgEarth
{
    /**
     * Drawable that renders lines using the GPU. It will fall back on rendering
     * OpenGL lines when shader-based rendering is unavailable.
     *
     * Note: Put LineDrawables under a LineGroup to share the shader program.
     * If you don't use a LineGroup, you must call installShader() on your
     * LineDrawable.
     *
     * Note: If you use this class you must have the oe_ViewportSize
     * uniform set. MapNode sets it automatically so any LineDrawable under
     * a MapNode is fine. Otherwise, use the osgEarth::InstallViewportSizeUniform
     * callback on your scene graph.
     *
     * Note: Try to use the provided functions whenever possible. If you do
     * need to access the underlying Geometry arrays, keep in mind that
     * the implementation uses double verts; i.e. each vertex appears twice
     * (to support the shader code).
     */
    class SDKVIS_EXPORT LineDrawable : public osg::Geometry
    {
    public:
        META_Node(osgEarth, LineDrawable);

        //! Create new LineDrawable in GL_LINE_STRIP mode
        LineDrawable();

        //! Create a new LineDrawable.
        //! @param[in ] mode GL line mode: GL_LINE_STRIP or GL_LINE_LOOP
        LineDrawable(GLenum mode);

        //! Copy constructor
        LineDrawable(const LineDrawable& rhs, const osg::CopyOp& copy);

        //! Width in pixels of the line
        void setLineWidth(float width_pixels);
        float getLineWidth() const { return _width; }

        //! Stippling pattern for the line (default is 0xFFFF)
        void setStipplePattern(GLushort pattern);
        GLushort getStipplePattern() const { return _pattern; }

        //! Stippling factor for the line (default is 1)
        void setStippleFactor(GLint factor);
        GLint getStippleFactor() const { return _factor; }

        //! Sets the overall color of the line
        void setColor(const osg::Vec4& color);
        const osg::Vec4& getColor() const { return _color; }

        //! GL mode (for serializer only; do not use)
        void setMode(GLenum mode);
        GLenum getMode() const { return _mode; }

        //! Append a vertex to the line
        void pushVertex(const osg::Vec3& vert);

        //! Set the value of a vertex at index i
        void setVertex(unsigned i, const osg::Vec3& vert);

        //! Gets the vertex at index i
        const osg::Vec3& getVertex(unsigned i) const;

        //! Sets the color of a vertex at index i
        void setColor(unsigned i, const osg::Vec4& color);

        //! Copy a vertex array into the drawable
        void importVertexArray(const osg::Vec3Array* verts);

        //! Copy a vertex attribute array into the drawable
        template<typename T>
        void importVertexAttribArray(unsigned location, const T* data);

        //! Allocate space for vertices
        void allocate(unsigned numVerts);

        //! Clears all data
        void clear();

        //! Number of vertices in the drawable
        unsigned getNumVerts() const;

        //! Number of vertices in the drawable
        unsigned size() const { return getNumVerts(); }

        //! Appends a vertex to an attribute array. Use this instead of adding
        //! to the array directly!
        template<typename T>
        void pushVertexAttrib(T* vaa, const typename T::ElementDataType& value);

        //! Pre-allocate space for vertices
        void reserve(unsigned size);

        //! Index of the first vertex to draw (default = 0)
        void setFirst(unsigned index);
        unsigned getFirst() const;

        //! Number of vertices to draw (default = 0, which means draw to the
        //! end of the line
        void setCount(unsigned count);
        unsigned getCount() const;

        //! Rebuild the primitive sets for this drawable. You MUST call this
        //! after adding new data to the drawable!
        void dirty();

        //! Install shaders to render the line drawable when GPU rendering is
        //! available. You only need to call this if you are not using a LineGroup.
        void installShader();

        //! Sets a line width on a custom stateset that will apply to
        //! all LineDrawables used with that state set.
        static void setLineWidth(osg::StateSet* stateSet, float value, int overrideFlags=osg::StateAttribute::ON);

        //! Install the line shaders on an arbitraty state set.
        static void installShader(osg::StateSet*);

    public:

        //! Binding location for "previous" vertex attribute (default = 9)
        static int PreviousVertexAttrLocation;

        //! Binding location for "next" vertex attribute (default = 10)
        static int NextVertexAttrLocation;

    private:
        GLenum _mode;
        bool _gpu;
        osg::Vec4 _color;
        GLint _factor;
        GLushort _pattern;
        float _width;
        unsigned _first;
        unsigned _count;
        osg::Vec3Array* _current;
        osg::Vec3Array* _previous;
        osg::Vec3Array* _next;
        osg::Vec4Array* _colors;

        void initialize();

        friend class LineGroup;

        unsigned actualVertsPerVirtualVert(unsigned) const;
        unsigned numVirtualVerts(const osg::Array*) const;
    };


    /**
     * Group for collecting multiple LineDrawables. If you put one or more
     * LineDrawables under a LineGroup, you do not need to call installShader()
     * on each LineDrawable because the LineGroup installs the shader for the
     * entire set.
     *
     * Note: LineGroup inherits from Geode (for now) to maintain support for
     * the OSG 3.4 MergeGeometryVisitor (which only works on Geodes). Once we
     * make OSG 3.6 the minimum supported version, we can change this to Group.
     */
    class SDKVIS_EXPORT LineGroup : public osg::Geode
    {
    public:
        META_Node(osgEarth, LineGroup);

        //! Construct a new line group and install line shaders
        LineGroup();

        //! Copy constructor
        LineGroup(const LineGroup& rhs, const osg::CopyOp& copy);

        //! Imports any GL line drawables from a node graph, converts them
        //! to LineDrawables, and adds them to this LineGroup.
        //! If you set removePrimitiveSets to true, it will remove all line-based
        //! primitive sets from the node after import.
        void import(osg::Node* node, bool removePrimitiveSets =false);

        //! Optimize the LineDrawables under this group for performance.
        //! Only call this after you finish adding drawables to your group.
        void optimize();

        //! Get child i as a LineDrawable
        LineDrawable* getLineDrawable(unsigned i);

    protected:
        //! destructor
        virtual ~LineGroup() { }
    };


    // Template implementations ..........................................

    template<typename T>
    void LineDrawable::pushVertexAttrib(T* vaa, const typename T::ElementDataType& value)
    {
        unsigned nvv = numVirtualVerts(vaa);
        unsigned num = actualVertsPerVirtualVert(nvv);
        for (unsigned i = 0; i<num; ++i)
            vaa->push_back(value);
    }

    template<typename T>
    void LineDrawable::importVertexAttribArray(unsigned location, const T* data)
    {
        T* vaa = osg::cloneType(data);
        setVertexAttribArray(location, vaa);
        for (unsigned i=0; i < data->getNumElements(); ++i)
            pushVertexAttrib(vaa, (*data)[i]);
    }

} // namespace osgEarth

#endif // OSGEARTH_VERSION_LESS_THAN(2,10,0)

#endif // SIMVIS_LINEDRAWABLE_H
