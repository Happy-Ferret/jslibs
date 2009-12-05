# the following two variables are used by the target "waf dist"
VERSION='2572'
APPNAME='jslibs'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

# Take care of absolute header paths in 'uselib' referenced libraries
import preproc
preproc.go_absolute_uselib = 1

def set_options(opt):

    import Options

    opt.tool_options('compiler_cxx')
    opt.add_option('--prefix', default=Options.default_prefix, dest='prefix',
                   help="installation prefix [Default: '%s']" % Options.default_prefix)
    opt.add_option('--enable-debug', action="store_true", default=False, dest='debug',
                   help="Enable debug compilation [Default: 'no']")
    opt.add_option('--enable-strong-optimization', action="store_true", default=False, dest='optimize',
                   help="Enable strong (and risky) optimisation [Default: 'no']")
    opt.add_option('--enable-strong-warnings', action="store_true", default=False, dest='warnings',
                   help="Enable strong compilation warnings [Default: 'no']")

def configure(conf):

    import Options, Utils

    # Check compiler
    conf.check_tool('gcc g++')

    # Check options
    if Options.options.debug:
        conf.env['BUILD_MODE'] = 'debug'
        variant = conf.env.copy()
        conf.set_env_name('debug', variant)
        variant.set_variant('debug')

        conf.setenv('debug')

    elif Options.options.optimize:
        conf.env['BUILD_MODE'] = 'optimize'
        variant = conf.env.copy()
        conf.set_env_name('optimize', variant)
        variant.set_variant('optimize')

        conf.setenv('optimize')

    else:
        conf.env['BUILD_MODE'] = 'default'

    # Check submodule configuration
    conf.sub_config('libs/js')
    conf.sub_config('src/jsio')
    conf.sub_config('src/jssqlite')
    conf.sub_config('src/jsz')
    conf.sub_config('src/jsiconv')
    conf.sub_config('src/jscrypt')
    conf.sub_config('src/jsfastcgi')
    conf.sub_config('src/jsimage')
    conf.sub_config('src/jsfont')
    conf.sub_config('src/tools')

    # Enable/Disable module
    conf.env['JL_ENABLED_MODULE'] = []
    conf.env['JL_DISABLED_MODULE'] = []

    conf.env['JL_ENABLED_MODULE'].append('jsstd')
    conf.env['JL_ENABLED_MODULE'].append('jstask')
    conf.env['JL_ENABLED_MODULE'].append('jsprotex')
    conf.env['JL_ENABLED_MODULE'].append('jstrimesh')

    if conf.env['HAVE_NSPR']:
        conf.env['JL_ENABLED_MODULE'].append('jsio')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jsio (missing NSPR library)')

    if conf.env['HAVE_SQLITE']:
        conf.env['JL_ENABLED_MODULE'].append('jssqlite')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jssqlite (missing Sqlite library)')

    if conf.env['HAVE_ZLIB_H']:
        conf.env['JL_ENABLED_MODULE'].append('jsz')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jsz (missing Zlib library)')

    if conf.env['HAVE_ICONV']:
        conf.env['JL_ENABLED_MODULE'].append('jsiconv')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jsiconv (missing iconv library)')

    if conf.env['HAVE_TOMCRYPT'] and conf.env['HAVE_MP_INIT']:
        conf.env['JL_ENABLED_MODULE'].append('jscrypt')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jscrypt (missing Tomcrypt or Tommath libraries)')

    if conf.env['HAVE_FCGX_ACCEPT_R']:
        conf.env['JL_ENABLED_MODULE'].append('jsfastcgi')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jsfastcgi (missing FastCGI library)')

    if conf.env['HAVE_ZLIB_H'] and conf.env['HAVE_PNG'] and conf.env['LIB_JPEG']:
        conf.env['JL_ENABLED_MODULE'].append('jsimage')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jsimage (missing Zlib or PNG or JPEG library)')

    if conf.env['HAVE_FREETYPE']:
        conf.env['JL_ENABLED_MODULE'].append('jsfont')
    else:
        conf.env['JL_DISABLED_MODULE'].append('jsfont (missing FreeType library)')
    
    # Add gcc Warnings
    conf.env.append_value('CFLAGS', Utils.to_list('-Wall'))
    conf.env.append_value('CXXFLAGS', Utils.to_list('-Wall -Wno-invalid-offsetof'))

    if Options.options.warnings:
        # TODO: Due to spider monkey: -Wshadow -Wno-unused-parameter -Wno-invalid-offsetof
        conf.env.append_value('CFLAGS', Utils.to_list('-std=c89 -pedantic -Werror -Wextra -Wmissing-prototypes -Wno-unused-parameter'))
        conf.env.append_value('CXXFLAGS', Utils.to_list('-Werror -Wextra -Wno-unused-parameter'))

    if Options.options.debug:
        conf.env.append_value('CFLAGS', Utils.to_list('-g3 -O0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -DDEBUG'))
        conf.env.append_value('CXXFLAGS', Utils.to_list('-g3 -O0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -DDEBUG'))

    elif Options.options.optimize:
        # Removed -felide-constructors (by default)
        conf.env.append_value('CFLAGS', Utils.to_list('-s -O3 -funroll-loops -fno-exceptions -fno-rtti'))
        conf.env.append_value('CXXFLAGS', Utils.to_list('-s -O3 -funroll-loops -fno-exceptions -fno-rtti'))

    else:
        conf.env.append_value('CFLAGS', Utils.to_list('-g0 -O2'))
        conf.env.append_value('CXXFLAGS', Utils.to_list('-g0 -O2'))

    print "Checking for fortran...just kidding!"

    def pretty_print_option(option):
        Utils.pprint('Normal', option.capitalize() + ": ", sep='')
        opt = getattr(Options.options, option)
        Utils.pprint('GREEN', 'yes') if opt else Utils.pprint('YELLOW', 'no')

    def pretty_print_env(env, color):
        Utils.pprint('Normal', env + ": ", sep='')
        Utils.pprint(color, " ".join(conf.env[env]))

    # Configuration summary
    Utils.pprint('Normal', "\n")
    Utils.pprint('Normal', "Configuration summary:")
    Utils.pprint('Normal', "======================")

    pretty_print_option('warnings');
    pretty_print_option('debug');
    pretty_print_option('optimize');

    pretty_print_env('CFLAGS', 'CYAN')
    pretty_print_env('CXXFLAGS', 'CYAN')

    pretty_print_env('JL_ENABLED_MODULE', 'GREEN')
    pretty_print_env('JL_DISABLED_MODULE', 'YELLOW')

from TaskGen import feature, before

@feature('cprogram', 'cshlib', 'cstaticlib')
@before('apply_link')
def redefine_liboutput(self):

    # Permit to have jz.so target instead of libjz.so
    import sys
    if sys.platform != 'win32':
        self.env.shlib_PATTERN = '%s.so'

def build(bld):

    bld.add_subdirs('libs/js')
    bld.add_subdirs('src/tools')

    bld.add_subdirs('libs/nedmalloc')

    bld.add_subdirs('src/host')
    bld.add_subdirs('src/jslang')
    bld.add_subdirs('src/jsdebug')

    for module in bld.env_of_name(bld.env.BUILD_MODE)['JL_ENABLED_MODULE']:
        bld.add_subdirs('src/' + module)

    bld.add_subdirs('src/jshost')

    # Change all tasks default env to current one [debug, optimize, ...]
    for obj in bld.all_task_gen[:]:

        # In case of debug/optimize compilation avoid default compilation
        if bld.env.BUILD_MODE != 'default':
            obj_new = obj.clone(bld.env.BUILD_MODE)

            # Keep task dependencies
            if obj_new.name == 'fileToCRes':
                bld.add_group()
            elif obj_new.name == 'xrdtocres':
                bld.add_group()
            obj.posted = True

    # QA feature support
    import qa
    bld.add_post_fun(qa.summary)

def qa(ctx):

    import Options, Scripting
    Options.options.__dict__['qa'] = True
    Scripting.commands += ['build']
