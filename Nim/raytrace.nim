
import math, times, streams, strutils

when defined(bigImgMT):
  {.experimental.}
  import threadpool, cpuinfo

# ---

const fov = 45.0
const maxDepth = 6

when defined(avgRuns):
  const runCount = 20
else:
  const runCount = 1

when defined(bigImg) or defined(bigImgMT):
  let jobCount = countProcessors() * 2
  const width  = 1280 * 8
  const height = 720 * 8
else:
  const width  = 1280
  const height = 720

# ---------- ---------- ---------- #

type Vec3 = tuple[x, y, z:float]

# ---

template zero(T:type Vec3): Vec3 = (0.0, 0.0, 0.0)

{.push inline noInit.}

proc `-`(v:Vec3): Vec3 = (-v.x, -v.y, -v.z)

proc `+`(a, b:Vec3): Vec3 = (a.x + b.x, a.y + b.y, a.z + b.z)
proc `-`(a, b:Vec3): Vec3 = (a.x - b.x, a.y - b.y, a.z - b.z)

proc `*`(a, b:Vec3): Vec3 = (a.x * b.x, a.y * b.y, a.z * b.z)
proc `*`(a:Vec3, b:float): Vec3 = (a.x * b, a.y * b, a.z * b)

proc `/`(a, b:Vec3): Vec3 = (a.x / b.x, a.y / b.y, a.z / b.z)
proc `/`(a:Vec3, b:float): Vec3 = (a.x / b, a.y / b, a.z / b)

proc `+=`(a:var Vec3, b:Vec3) =
  a.x += b.x
  a.y += b.y
  a.z += b.z

proc dot(a, b:Vec3): float = (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
proc normalize(v:Vec3): Vec3 = v / sqrt(dot(v, v))

converter toVec3[T:SomeNumber](v:tuple[x:T, y:T, z:T]): Vec3 =
  (float(v.x), float(v.y), float(v.z))

{.pop.}

# ---------- ---------- ---------- #

type
  Ray = object
    pos: Vec3
    dir: Vec3
  
  Light = ref object
    pos: Vec3
    color: Vec3
  
  Sphere = ref object
    pos: Vec3
    color: Vec3
    radius: float
    refl: float
    tran: float
  
  Scene = ref object
    objects: seq[Sphere]
    lights: seq[Light]

# ---

proc new(T:type Scene): Scene =
  result.new()
  result.objects = @[]
  result.lights = @[]

# ---

{.push noInit.}

proc normal(sphere:Sphere, pos:Vec3): Vec3 {.inline.} =
  return normalize(pos - sphere.pos)


proc intersect(sphere:Sphere, ray:Ray): bool =
  let d = sphere.pos - ray.pos
  let a = dot(d, ray.dir)
  if a < 0: # opposite direction
    return false

  let b = dot(d, d) - (a * a)
  let r = sphere.radius * sphere.radius
  if b > r: # perpendicular > r
    return false
  
  return true


proc intersect(sphere:Sphere, ray:Ray, distance:var float): bool =
  let d = sphere.pos - ray.pos
  let a = dot(d, ray.dir)
  if a < 0: # opposite direction
    distance = 0
    return false
  
  let b = dot(d, d) - (a * a)
  let r = sphere.radius * sphere.radius
  if b > r: # perpendicular > r
    distance = 0
    return false
  
  let c = sqrt(r - b)
  let near = a - c
  let far  = a + c
  # near < 0 means ray starts inside
  distance = if near < 0: far else: near
  
  return true


proc trace(ray:Ray, scene:Scene, depth:int): Vec3 = 
  var nearest = Inf
  var obj: Sphere
  
  # search the scene for nearest intersection
  for o in scene.objects:
    var distance: float
    if o.intersect(ray, distance):
      if distance < nearest:
        nearest = distance
        obj = o
  
  if obj == nil:
    return Vec3.zero
  
  let hitPoint = ray.pos + (ray.dir * nearest)
  var normal = obj.normal(hitPoint)
  var inside = false
  
  if dot(normal, ray.dir) > 0:
    inside = true
    normal = -normal
  
  var color = Vec3.zero
  let reflRatio = obj.refl
  
  for light in scene.lights:
    let lightDir = normalize(light.pos - hitPoint)
    let tr = Ray(pos:hitPoint + (normal * 1.0e-5), dir:lightDir)
    
    # go through the scene check whether we're blocked from the lights
    var blocked = false
    for o in scene.objects:
      if o.intersect(tr):
        blocked = true
        break
    
    if not blocked:
      color += light.color * max(0, dot(normal, lightDir)) *
        obj.color * (1 - reflRatio)
  
  let rayNormDot = dot(ray.dir, normal)
  let facing = max(0, -rayNormDot)
  let fresnel = reflRatio + ((1 - reflRatio) * pow((1 - facing), 5))
  
  # compute reflection
  if depth < maxDepth and reflRatio > 0:
    let reflDir = ray.dir + (normal * 2 * rayNormDot * -1)
    let tr = Ray(pos:hitPoint + (normal * 1.0e-5), dir:reflDir)
    color += tr.trace(scene, depth + 1) * fresnel
  
  # compute refraction
  if depth < maxDepth and obj.tran > 0:
    const iorDefault = 1 / 1.5
    let ior = if inside: iorDefault else: 1.5
    let ce = dot(ray.dir, normal) * -1
    let eta = 1 / ior
    let gf = (ray.dir + normal * ce) * eta
    let sin_t1_2 = 1 - (ce * ce)
    let sin_t2_2 = sin_t1_2 * (eta * eta)
    
    if sin_t2_2 < 1:
      let gc = normal * sqrt(1 - sin_t2_2)
      let refrDir = gf - gc
      let tr = Ray(pos:hitPoint - (normal * 1.0e-4), dir:refrDir)
      color += tr.trace(scene, depth + 1) * (1 - fresnel) * obj.tran
  
  return color

{.pop.}

# ---------- ---------- ---------- #

const
  pixmapSize = width * height * 3

type
  Pixmap = array[pixmapSize, byte]

# ---

proc renderRegion(pixmap:ptr Pixmap, scene:Scene, w, h:float, sx, sy, ex, ey:int) =
  let wf: float = width
  let hf: float = height
  
  for y in sy .. ey:
    for x in sx .. ex:
      let dir: Vec3 = (
        x: ((float(x) - (wf / 2)) / wf) * w,
        y: (((hf / 2) - float(y)) / hf) * h,
        z: -1.0
      ).normalize()
      
      let ray = Ray(pos:Vec3.zero, dir:dir)
      let pixel = ray.trace(scene, 0)
      let index = (x * 3) + (y * width * 3)
      
      pixmap[index]   = byte min(pixel.x * 255, 255)
      pixmap[index+1] = byte min(pixel.y * 255, 255)
      pixmap[index+2] = byte min(pixel.z * 255, 255)


proc render(pixmap:ref Pixmap, scene:Scene) =
  # cast to unsafe ptr type to avoid the 'parallel' macro
  # from deep-copying the pixmap for earch `renderRegion` call
  let pm = cast[ptr Pixmap](pixmap)
  
  let h = tan(((fov / 360) * (2 * Pi)) / 2) * 2
  let w = h * width / height
  
  when not defined(bigImgMT):
    renderRegion(pm, scene, w, h, 0, 0, <width, <height)
  else:
    let sizeW = int(width / jobCount)
    let sizeH = int(height / jobCount)
    parallel:
      for y in 0 .. <jobCount:
        for x in 0 .. <jobCount:
          let sx = x * sizeW
          let sy = y * sizeH
          let ex = sx + <sizeW
          let ey = sy + <sizeH
          spawn renderRegion(pm, scene, w, h, sx, sy, ex, ey)

# ---------- ---------- ---------- #

proc main =
  ## raytraces a simple scene and saves an image
  let scene = new Scene
  let image = new Pixmap
  
  # add geometry
  scene.objects.add Sphere(pos:(0, -10002, -20), color:(0.8, 0.8, 0.8), radius:10000)
  scene.objects.add Sphere(pos:( 0,  2, -20), color:(0.8, 0.5, 0.5), radius:4, refl:0.5)
  scene.objects.add Sphere(pos:( 5,  0, -15), color:(0.3, 0.8, 0.8), radius:2, refl:0.2)
  scene.objects.add Sphere(pos:(-5,  0, -15), color:(0.3, 0.5, 0.8), radius:2, refl:0.2)
  scene.objects.add Sphere(pos:(-2, -1, -10), color:(0.1, 0.1, 0.1), radius:1, refl:0.1, tran:0.8)
  
  # add light
  scene.lights.add Light(pos:(-10, 20, 30), color:(2, 2, 2))
  
  # run test
  let begin = epochTime()
  for i in 1 .. runCount: image.render(scene)
  let finish = epochTime()
  
  # print results
  var elapsed = (finish - begin)
  when defined(avgRuns): elapsed /= runCount
  echo "Seconds: ", elapsed.formatFloat(ffDecimal, 3)
  
  when not defined(noSave):
    # save image
    let fs = newFileStream("image.rgb", fmWrite)
    fs.writeData(addr image[], pixmapSize)
    fs.close()

# ---

when isMainModule:
  main() # run program
