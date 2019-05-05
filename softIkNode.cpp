#include <math.h>

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

class softIk : public MPxNode
{
	public:
		softIk();
		~softIk() override;

		MStatus compute(const MPlug& plug, MDataBlock& data) override;

		static void* creator();
		static MStatus initialize();

		static MObject totalLength;
		static MObject currentLength;
		static MObject softStart;
		static MObject outOffset;

		static MTypeId id;
};

MTypeId softIk::id(0x130a01);
MObject softIk::totalLength;
MObject softIk::currentLength;
MObject softIk::softStart;
MObject softIk::outOffset;

softIk::softIk() {}
softIk::~softIk() {}

void* softIk::creator()
{
	return new softIk();
}

MStatus softIk::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus status;

	if (plug == outOffset)
	{
		MDataHandle totalLengthHandle = data.inputValue(totalLength, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		MDataHandle currentLengthHandle = data.inputValue(currentLength, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		MDataHandle softStartHandle = data.inputValue(softStart, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		double totalLength = totalLengthHandle.asDouble();
		double curLength = currentLengthHandle.asDouble();
		double softStart = softStartHandle.asDouble();
		double triggerDist = totalLength - softStart;

		double value = 0;
		if (curLength >= triggerDist && softStart > 0)
		{
			value = (1 - exp(( - (curLength - triggerDist)) / softStart) + triggerDist) - curLength;
		}

		MDataHandle outOffsetHandle = data.outputValue(outOffset, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		outOffsetHandle.set(value);
		data.setClean(plug);
		return MS::kSuccess;
	}

	return MS::kUnknownParameter;
}

MStatus softIk::initialize()
{
	MStatus status;
	MFnNumericAttribute nAttr;

	totalLength = nAttr.create("totalLength", "tl", MFnNumericData::kDouble, 0.0);
	nAttr.setChannelBox(true);
	nAttr.setKeyable(true);
	nAttr.setStorable(true);

	currentLength = nAttr.create("currentLength", "cl", MFnNumericData::kDouble, 0.0);
	nAttr.setChannelBox(true);
	nAttr.setKeyable(true);
	nAttr.setStorable(true);

	softStart = nAttr.create("softStart", "s", MFnNumericData::kDouble, 1.0);
	nAttr.setChannelBox(true);
	nAttr.setKeyable(true);
	nAttr.setStorable(true);

	outOffset = nAttr.create("outOffset", "o", MFnNumericData::kDouble, 0.0);
	nAttr.setStorable(false);

	// Inputs
	status = addAttribute(totalLength);
	if (!status) { status.perror("addAttribute(totalLength)"); return status; }
	status = addAttribute(currentLength);
	if (!status) { status.perror("addAttribute(currentLength)"); return status; }
	status = addAttribute(softStart);
	if (!status) { status.perror("addAttribute(softStart)"); return status; }

	// Outputs
	status = addAttribute(outOffset);
	if (!status) { status.perror("addAttribute(outOffset)"); return status; }

	// Inputs affecting outputs
	status = attributeAffects(totalLength, outOffset);
	if (!status) { status.perror("attributeAffects(totalLength, outOffset)"); return status; }
	status = attributeAffects(currentLength, outOffset);
	if (!status) { status.perror("attributeAffects(currentLength, outOffset)"); return status; }
	status = attributeAffects(softStart, outOffset);
	if (!status) { status.perror("attributeAffects(softStart, outOffset)"); return status; }

	return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, "https://github.com/PaulSchweizer", "1.0", "Any");

	status = plugin.registerNode(
		"softIk",
		softIk::id,
		&softIk::creator,
		&softIk::initialize);
	if (!status) {
		status.perror("registerNode");
	}
	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode(softIk::id);
	if (!status) {
		status.perror("deregisterNode");
	}
	return status;
}
