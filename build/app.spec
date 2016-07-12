# -*- mode: python -*-

block_cipher = None

image_files = [('../images/*.png', 'images'), ('../images/*.ico', 'images')]

a = Analysis(['..\\app.py'],
             pathex=['C:\\code\\curve_tracer_gui\\build'],
             binaries=None,
             datas=image_files,
             hiddenimports=[],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher)
pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)
exe = EXE(pyz,
          a.scripts,
          exclude_binaries=True,
          name='curve-tracer',
          debug=False,
          strip=False,
          upx=True,
          console=False )
coll = COLLECT(exe,
               a.binaries,
               a.zipfiles,
               a.datas,
               strip=False,
               upx=True,
               name='curve-tracer')
