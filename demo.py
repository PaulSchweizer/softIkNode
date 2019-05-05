from maya import cmds


cmds.file(new=True, f=True)

if cmds.pluginInfo("softIkNode", q=True, loaded=True):
    cmds.unloadPlugin("softIkNode")
cmds.loadPlugin("softIkNode")

# Regular rotate plane ik setup
#
joint1 = cmds.joint(p=[0, 0, 0])
joint2 = cmds.joint(p=[2, 0, 4])
joint3 = cmds.joint(p=[0, 0, 8])

ctl = cmds.circle()
cmds.setAttr("{0}.tz".format(ctl[0]), 8)
cmds.makeIdentity(ctl[0], apply=True, t=True)

ikh = cmds.ikHandle(sj=joint1, ee=joint3, sol="ikRPsolver")
cmds.parent(ikh[0], ctl[0])

# Soft Ik
#
softik = cmds.createNode("softIk", n="softik")

offsetAim = cmds.createNode("transform", n="offsetAim")
cmds.delete(cmds.pointConstraint(ctl, offsetAim))
cmds.parent(offsetAim, ctl[0])
cmds.makeIdentity(offsetAim, apply=True, t=True)

dist = cmds.createNode("distanceBetween", n="distance")
start = cmds.createNode("transform", n="start")
end = cmds.createNode("transform", n="end")
cmds.pointConstraint(ctl[0], end)
cmds.connectAttr(
    "{0}.worldMatrix[0]".format(start), "{0}.inMatrix1".format(dist))
cmds.connectAttr(
    "{0}.worldMatrix[0]".format(end), "{0}.inMatrix2".format(dist))
cmds.connectAttr(
    "{0}.distance".format(dist), "{0}.currentLength".format(softik))

# The joint lengths are hardcoded for simplicity
#
cmds.setAttr("{0}.totalLength".format(softik), 4.472 * 2)

offset = cmds.createNode("transform", n="offset")
cmds.parent(offset, offsetAim)
cmds.delete(cmds.pointConstraint(ctl, offset))
cmds.parent(ikh[0], offset)
cmds.makeIdentity(offset, apply=True, t=True)

# Soft ik drives the offset of the ik handle
#
cmds.aimConstraint(joint1, offsetAim, mo=True)
cmds.connectAttr("{0}.outOffset".format(softik), "{0}.tz".format(offset))

# Drive the softness of the ik through an attr on the ctl
#
cmds.addAttr(ctl[0], ln="soft", at="bool", dv=True, k=True)
cmds.connectAttr("{0}.soft".format(ctl[0]), "{0}.softStart".format(softik))

cmds.select(ctl[0])
