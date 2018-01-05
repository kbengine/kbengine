# -*- coding: utf-8 -*-
#
"""
"""
from KBEDebug import *
import d_skills

from skills.SkillAttack import SkillAttack

_g_skills = {}

def onInit():
	"""
	init skills.
	"""
	for key, datas in d_skills.datas.items():
		script = datas['script']
		scriptinst = eval(script)()
		_g_skills[key] = scriptinst
		scriptinst.loadFromDict(datas)
		
def getSkill(skillID):
	return _g_skills.get(skillID)