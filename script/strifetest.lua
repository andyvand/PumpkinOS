function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

lib = pit.loadlib("liblsdl3")

if not lib then
  print("could not load a display lib")
  pit.finish(0)
  return
end

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  density  = 144,
  width    = 640,
  height   = 480,
  abgr     = false,
  depth    = 16,
  hdepth   = 16,
  launcher = "Strife",
}
