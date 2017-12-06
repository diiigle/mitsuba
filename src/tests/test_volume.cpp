/*
    This file is part of Mitsuba, a physically based rendering system.

    Copyright (c) 2007-2014 by Wenzel Jakob and others.

    Mitsuba is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Mitsuba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <mitsuba/render/testcase.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/core/fstream.h>
#include <mitsuba/render/volume.h>

MTS_NAMESPACE_BEGIN

// forward declaration of GridDataSource::EVolumeType
class GridDataSource {
    public:
    enum EVolumeType {
        EFloat32 = 1,
        EFloat16 = 2,
        EUInt8 = 3,
        EQuantizedDirections = 4
    };
};

class TestVolumeDataSources : public TestCase {
public:
    MTS_BEGIN_TESTCASE()
    MTS_DECLARE_TEST(test01_Constvolume)
    MTS_DECLARE_TEST(test02_Gridvolume)
    MTS_END_TESTCASE()


    /**
     * @brief Loads a `constvolume` plugin, reads a value, sets a value and reads it back
     */
    void test01_Constvolume() {

        //float
        Properties floatProps("constvolume");
        floatProps.setFloat("value", 42.0);

        ref<VolumeDataSource> floatSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                createObject(MTS_CLASS(VolumeDataSource), floatProps));

        assert(floatSource->supportsFloatLookups());
        assert(floatSource->supportsFloatEdits());

        assertEqualsEpsilon(floatSource->lookupFloat(Point(0,0,0)), 42.0, 1e-7);
        floatSource->editFloat(Point(1,1,1), 1337.0f);
        assertEqualsEpsilon(floatSource->lookupFloat(Point(0,0,0)), 1337.0, 1e-7);

        //spectrum
        Properties spectrumProps("constvolume");
        spectrumProps.setSpectrum("value", Spectrum(42.0));

        ref<VolumeDataSource> spectrumSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                createObject(MTS_CLASS(VolumeDataSource), spectrumProps));

        assert(spectrumSource->supportsSpectrumLookups());
        assert(spectrumSource->supportsSpectrumEdits());

        assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(0,0,0)), Spectrum(42.0), 1e-7);
        spectrumSource->editSpectrum(Point(1,1,1), Spectrum(1337.0f));
        assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(0,0,0)), Spectrum(1337.0), 1e-7);


        //vectors
        Properties vectorProps("constvolume");
        vectorProps.setVector("value", Vector(1,0,0));

        ref<VolumeDataSource> vectorSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                createObject(MTS_CLASS(VolumeDataSource), vectorProps));

        assert(vectorSource->supportsVectorLookups());
        assert(vectorSource->supportsVectorEdits());

        assertEqualsEpsilon(vectorSource->lookupVector(Point(0,0,0)), Vector(1,0,0), 1e-7);
        vectorSource->editVector(Point(1,1,1), normalize(Vector(1,1,1)));
        assertEqualsEpsilon(vectorSource->lookupVector(Point(0,0,0)), normalize(Vector(1,1,1)), 1e-7);

    }

    void test02_Gridvolume() {
        /**
         * Single floats stored as float32
         */
        {
            Properties floatProps("gridvolume");

            ref<FileStream> floatSourceFile = writeNullVolumeGrid(GridDataSource::EVolumeType::EFloat32, 1, TAABB<Point3i>(Point3i(0),Point3i(10)));
            floatProps.setString("filename", floatSourceFile->getPath().c_str());
            floatProps.setBoolean("editable", true);

            ref<VolumeDataSource> floatSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                    createObject(MTS_CLASS(VolumeDataSource), floatProps));

            assert(floatSource->supportsFloatLookups());
            assert(floatSource->supportsFloatEdits());

            assertEqualsEpsilon(floatSource->lookupFloat(Point(0,0,0)), 0.0, 1e-7);

            floatSource->editFloat(Point(1,1,1), 1337.0f);

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(floatSource->lookupFloat(Point(0,1,1)), 0.0, 1e-7);
            assertEqualsEpsilon(floatSource->lookupFloat(Point(1,1,1)), 1337.0, 1e-7);
            assertEqualsEpsilon(floatSource->lookupFloat(Point(2,1,1)), 0.0, 1e-7);
        }

        /**
         * Single floats stored as uint8
         */
        {
            Properties floatProps("gridvolume");

            ref<FileStream> floatSourceFile = writeNullVolumeGrid(GridDataSource::EVolumeType::EUInt8, 1, TAABB<Point3i>(Point3i(0),Point3i(10)));
            floatProps.setString("filename", floatSourceFile->getPath().c_str());
            floatProps.setBoolean("editable", true);

            ref<VolumeDataSource> floatSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                    createObject(MTS_CLASS(VolumeDataSource), floatProps));

            assert(floatSource->supportsFloatLookups());
            assert(floatSource->supportsFloatEdits());

            assertEqualsEpsilon(floatSource->lookupFloat(Point(0,0,0)), 0.0, 1e-7);

            floatSource->editFloat(Point(1,1,1), 0.5f);

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(floatSource->lookupFloat(Point(0,1,1)), 0.0, 1e-7);
            assertEqualsEpsilon(floatSource->lookupFloat(Point(1,1,1)), 0.5, (0.5/255)+1e-7); // less than half the uint8 precision gap
            assertEqualsEpsilon(floatSource->lookupFloat(Point(2,1,1)), 0.0, 1e-7);

            floatSource->editFloat(Point(1,2,1), 127.0/255);

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(floatSource->lookupFloat(Point(0,2,1)), 0.0, 1e-7);
            assertEqualsEpsilon(floatSource->lookupFloat(Point(1,2,1)), 127.0/255, 4e-5);
            assertEqualsEpsilon(floatSource->lookupFloat(Point(2,2,1)), 0.0, 1e-7);
        }
        
        /**
         * Spectrum stored as float32
         */
        {
            Properties spectrumProps("gridvolume");

            ref<FileStream> spectrumSourceFile = writeNullVolumeGrid(GridDataSource::EVolumeType::EFloat32, 3, TAABB<Point3i>(Point3i(0),Point3i(10)));
            spectrumProps.setString("filename", spectrumSourceFile->getPath().c_str());
            spectrumProps.setBoolean("editable", true);

            ref<VolumeDataSource> spectrumSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                    createObject(MTS_CLASS(VolumeDataSource), spectrumProps));

            assert(spectrumSource->supportsSpectrumLookups());
            assert(spectrumSource->supportsSpectrumEdits());

            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(0,0,0)), Spectrum(0.0), 1e-7);

            Spectrum editValue(0.0f);
            editValue.fromLinearRGB(13.,3.,7.);

            spectrumSource->editSpectrum(Point(1,1,1), editValue);

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(0,1,1)), Spectrum(0.0), 1e-7);
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(1,1,1)), editValue, 1e-7);
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(2,1,1)), Spectrum(0.0), 1e-7);
        }

        /**
         * Spectrum stored as uint8
         */
        {
            Properties spectrumProps("gridvolume");

            ref<FileStream> spectrumSourceFile = writeNullVolumeGrid(GridDataSource::EVolumeType::EUInt8, 3, TAABB<Point3i>(Point3i(0),Point3i(10)));
            spectrumProps.setString("filename", spectrumSourceFile->getPath().c_str());
            spectrumProps.setBoolean("editable", true);

            ref<VolumeDataSource> spectrumSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                    createObject(MTS_CLASS(VolumeDataSource), spectrumProps));

            assert(spectrumSource->supportsSpectrumLookups());
            assert(spectrumSource->supportsSpectrumEdits());

            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(0,0,0)), Spectrum(0.0), 1e-7);

            Spectrum editValue(0.0f);
            editValue.fromLinearRGB(0.13,0.3,0.7);

            spectrumSource->editSpectrum(Point(1,1,1), editValue); // any float

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(0,1,1)), Spectrum(0.0), 1e-7);
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(1,1,1)), editValue, (0.5/255)+1e-7); // less than half the uint8 precision gap
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(2,1,1)), Spectrum(0.0), 1e-7);

            spectrumSource->editSpectrum(Point(1,2,1), Spectrum(127.0/255));

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(0,2,1)), Spectrum(0.0), 1e-7);
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(1,2,1)), Spectrum(127.0/255), 4e-5);
            assertEqualsEpsilon(spectrumSource->lookupSpectrum(Point(2,2,1)), Spectrum(0.0), 1e-7);
        }
        
        /**
         * Vector stored as float32
         */
        {
            Properties vectorProps("gridvolume");

            ref<FileStream> vectorSourceFile = writeNullVolumeGrid(GridDataSource::EVolumeType::EFloat32, 3, TAABB<Point3i>(Point3i(0),Point3i(10)));
            vectorProps.setString("filename", vectorSourceFile->getPath().c_str());
            vectorProps.setBoolean("editable", true);

            ref<VolumeDataSource> vectorSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                    createObject(MTS_CLASS(VolumeDataSource), vectorProps));

            assert(vectorSource->supportsVectorLookups());
            assert(vectorSource->supportsVectorEdits());

            assertEqualsEpsilon(vectorSource->lookupVector(Point(0,0,0)), Vector(0.0), 1e-7);

            Vector editValue = normalize(Vector(13.,3.,7.));

            vectorSource->editVector(Point(1,1,1), editValue);

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(vectorSource->lookupVector(Point(0,1,1)), Vector(0.0), 1e-7);
            assertEqualsEpsilon(vectorSource->lookupVector(Point(1,1,1)), editValue, 1e-7);
            assertEqualsEpsilon(vectorSource->lookupVector(Point(2,1,1)), Vector(0.0), 1e-7);
        }

        /**
         * Vector stored as quantizedDirections(uint8)
         */
        {
            Properties vectorProps("gridvolume");

            ref<FileStream> vectorSourceFile = writeNullVolumeGrid(GridDataSource::EVolumeType::EQuantizedDirections, 2, TAABB<Point3i>(Point3i(0),Point3i(10)));
            vectorProps.setString("filename", vectorSourceFile->getPath().c_str());
            vectorProps.setBoolean("editable", true);

            ref<VolumeDataSource> vectorSource = static_cast<VolumeDataSource *> (PluginManager::getInstance()->
                    createObject(MTS_CLASS(VolumeDataSource), vectorProps));

            assert(vectorSource->supportsVectorLookups());
            assert(vectorSource->supportsVectorEdits());

            assertEqualsEpsilon(vectorSource->lookupVector(Point(0,0,0)), Vector(0.0,0.0,1.0), 1e-7);

            Vector editValue = normalize(Vector(13.,3.,7.));

            vectorSource->editVector(Point(1,1,1), editValue); // any float

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(vectorSource->lookupVector(Point(0,1,1)), Vector(0.0,0.0,1.0), 1e-7);
            assertEqualsEpsilon(vectorSource->lookupVector(Point(1,1,1)), editValue, (1.0/255)+1e-3); // less than half the uint8 precision gap, but nonlinear [-1,1]
            assertEqualsEpsilon(vectorSource->lookupVector(Point(2,1,1)), Vector(0.0,0.0,1.0), 1e-7);

            Vector halfVector = sphericalDirection(0.25f * M_PI, M_PI); //exactly 64,128 in QuantizedDirection notation
            vectorSource->editVector(Point(1,2,1), halfVector);

            //test for write operatation and nearby data corruption
            assertEqualsEpsilon(vectorSource->lookupVector(Point(0,2,1)), Vector(0.0,0.0,1.0), 1e-7);
            assertEqualsEpsilon(vectorSource->lookupVector(Point(1,2,1)), halfVector, 9e-3);
            assertEqualsEpsilon(vectorSource->lookupVector(Point(2,2,1)), Vector(0.0,0.0,1.0), 1e-7);
        }
        
    }


    ref<FileStream> writeNullVolumeGrid(GridDataSource::EVolumeType type, int channels, TAABB<Point3i> bounds) {
        assert(bounds.isValid());

        ref<FileStream> fs = FileStream::createTemporary();

        char header[] = {'V','O','L'};
        fs->write(&header, sizeof(header));

        fs->writeChar(3);
        fs->writeInt(type);

        int xres = (bounds.max.x - bounds.min.x)+1;
        int yres = (bounds.max.y - bounds.min.y)+1;
        int zres = (bounds.max.z - bounds.min.z)+1;
        fs->writeInt(xres);
        fs->writeInt(yres);
        fs->writeInt(zres);

        fs->writeInt(channels);

        fs->writeSingle(bounds.min.x);
        fs->writeSingle(bounds.min.y);
        fs->writeSingle(bounds.min.z);
        fs->writeSingle(bounds.max.x);
        fs->writeSingle(bounds.max.y);
        fs->writeSingle(bounds.max.z);

        size_t size = xres * yres * zres * channels;
        void * data;

        switch((GridDataSource::EVolumeType)type) {
            case GridDataSource::EVolumeType::EFloat32: {
                std::vector<float> vector(size, 0);
                data = vector.data();
                size *= sizeof(float);
            }
            break;
            
            case GridDataSource::EVolumeType::EQuantizedDirections:
            case GridDataSource::EVolumeType::EUInt8: {
                std::vector<unsigned char> vector(size, 0);
                data = vector.data();
                size *= sizeof(unsigned char);
            }
            break;

            default:
                Log(EError, "EVolumeType not supported");
                break;
        };

        fs->write(data, size);
        fs->flush();

        return fs;
    }
};

MTS_EXPORT_TESTCASE(TestVolumeDataSources, "Testcase for volume data sources")

MTS_NAMESPACE_END
