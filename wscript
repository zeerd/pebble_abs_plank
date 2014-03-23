
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#
import json

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    def generate_appinfo(task):
        src = task.inputs[0].abspath()
        tgt = task.outputs[0].abspath()

        json_data=open(src)
        data = json.load(json_data)

        f = open(tgt,'w')
        f.write('#ifndef __APPINFO_H__\n')
        f.write('#define __APPINFO_H__\n')
        f.write('#define VERSION_LABEL "' + data["versionLabel"] + '"\n') 
        f.write('#endif\n')
        f.close()

    ctx(
        rule   = generate_appinfo,
        source = 'appinfo.json',
        target = 'generated/appinfo.h',
    )

    ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
                    target='pebble-app.elf')

    ctx.pbl_bundle(elf='pebble-app.elf',
                   js=ctx.path.ant_glob('src/js/**/*.js'))
