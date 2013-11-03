
import Math, Times, StrUtils, Streams

# ---

type Vec3 = tuple
  x, y, z: float

template new(T:type Vec3, x, y, z:float): Vec3 = (x, y, z)
template zero(T:type Vec3): Vec3 = (0.0, 0.0, 0.0)

template dot(T:type Vec3, a, b:Vec3): float = (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
template magnitude(T:type Vec3, v:Vec3): float = sqrt(Vec3.dot(v, v))
template normalize(T:type Vec3, v:Vec3): Vec3 = v / sqrt(Vec3.dot(v, v))

# neg
proc `-`(v:Vec3): Vec3 {.inline, noInit, noSideEffect.} =
  return Vec3.new(-v.x, -v.y, -v.z)

# +, -
proc `+`(a, b:Vec3): Vec3 {.inline, noInit, noSideEffect.} =
  return (a.x + b.x, a.y + b.y, a.z + b.z)

proc `-`(a, b:Vec3): Vec3 {.inline, noInit, noSideEffect.} =
  return (a.x - b.x, a.y - b.y, a.z - b.z)

# *
proc `*`(a, b:Vec3): Vec3 {.inline, noInit, noSideEffect.} =
  return (a.x * b.x, a.y * b.y, a.z * b.z)

proc `*`(a:Vec3, b:float): Vec3 {.inline, noInit, noSideEffect.} =
  return Vec3.new(a.x * b, a.y * b, a.z * b)

proc `*`(a:float, b:Vec3): Vec3 {.inline, noInit, noSideEffect.} =
  return Vec3.new(a * b.x, a * b.y, a * b.z)

# /
proc `/`(a, b:Vec3): Vec3 {.inline, noInit, noSideEffect.} = 
  return (a.x / b.x, a.y / b.y, a.z / b.z)

proc `/`(a:Vec3, b:float): Vec3 {.inline, noInit, noSideEffect.} =
  return Vec3.new(a.x / b, a.y / b, a.z / b)

# +=, -=
proc `+=`(this:var Vec3, v:Vec3) {.inline, noInit, noSideEffect.} =
  this.x += v.x
  this.y += v.y
  this.z += v.z

proc `-=`(this:var Vec3, v:Vec3) {.inline, noInit, noSideEffect.} =
  this.x -= v.x
  this.y -= v.y
  this.z -= v.z


type Ray {.pure, final.} = object
  org, dir: Vec3

# ---

type Sphere {.pure, final.} = ref object
  center: Vec3
  radius: float
  color: Vec3
  reflection: float
  transparency: float

proc new(T:type Sphere, c:Vec3, r:float, clr:Vec3, refl:float = 0, trans:float = 0): Sphere {.inline, noInit, noSideEffect.} =
  result.new()
  result.center = c
  result.radius = r
  result.color = clr
  result.reflection = refl
  result.transparency = trans

proc normal(this:Sphere, pos:Vec3): Vec3 {.inline, noInit.} =
  return Vec3.normalize(pos - this.center)

proc intersect(this:Sphere, ray:Ray): bool {.noInit, noSideEffect.} =
  let l = this.center - ray.org
  let a = Vec3.dot(l, ray.dir)
  if a < 0: # opposite direction
    return false

  let b2 = Vec3.dot(l, l) - (a * a)
  let r2 = this.radius * this.radius
  if b2 > r2: # perpendicular > r
    return false
  
  return true

proc intersect(this:Sphere, ray:Ray, distance:var float): bool {.noInit.} =
  distance = 0
  
  let l = this.center - ray.org
  let a = Vec3.dot(l, ray.dir)
  if a < 0: # opposite direction
    return false
  
  let b2 = Vec3.dot(l, l) - (a * a)
  let r2 = this.radius * this.radius
  if b2 > r2: # perpendicular > r
    return false
  
  let c = sqrt(r2 - b2)
  let near = a - c
  let far  = a + c
  # near < 0 means ray starts inside
  distance = if near < 0: far else: near
  
  return true

proc reflectionRatio(this:Sphere): float {.inline, noInit, noSideEffect.} = 
  return this.reflection

# ---

type Light {.pure, final.} = ref object
  position: Vec3
  color: Vec3

proc new(T:type Light, position, color:Vec3): Light {.inline, noInit, noSideEffect.} =
  result.new()
  result.position = position
  result.color = color

# ---

type Scene {.pure, final.} = ref object
  objects: seq[Sphere]
  lights: seq[Light]

proc new(T:type Scene): Scene {.inline, noInit, noSideEffect.} = 
  result.new()
  #result.objects = newSeq[Sphere.type]()
  #result.lights = newSeq[Light.type]()

# ---

const
  width = 1280
  height = 720
  fov = 45.0
  maxDepth = 6

proc trace (ray:Ray, scene:Scene, depth:int): Vec3 = 
  var nearest = inf
  var obj: Sphere
  
  # search the scene for nearest intersection
  for o in scene.objects:
    var distance = inf
    if o.intersect(ray, distance):
      if distance < nearest:
        nearest = distance
        obj = o
  
  if obj == nil:
    return Vec3.zero
  
  let point_of_hit = ray.org + (ray.dir * nearest)
  var normal = obj.normal(point_of_hit)
  var inside = false
  
  if Vec3.dot(normal, ray.dir) > 0:
    inside = true
    normal = -normal
  
  var color = Vec3.zero
  let reflection_ratio = obj.reflection_ratio()
  
  for l in scene.lights:
    let light_direction = Vec3.normalize(l.position - point_of_hit)
    var r: Ray
    r.org = point_of_hit + (normal * 1.0e-5)
    r.dir = light_direction
    
    # go through the scene check whether we're blocked from the lights
    var blocked = false
    for o in scene.objects:
      if o.intersect(r):
        blocked = true
        break
    
    if not blocked:
      color += l.Color * max(0, Vec3.dot(normal, light_direction)) *
        obj.color * (1.0 - reflection_ratio)
  
  let
    rayNormDot = Vec3.dot(ray.dir, normal)
    facing = max(0, -rayNormDot)
    fresneleffect = reflection_ratio + ((1 - reflection_ratio) * pow((1 - facing), 5))

  # compute reflection
  if depth < maxDepth and reflection_ratio > 0:
    let reflection_direction = ray.dir + (normal * 2 * rayNormDot * -1.0)
    var r: Ray
    r.org = point_of_hit + (normal * 1.0e-5)
    r.dir = reflection_direction
    color += trace(r, scene, depth + 1) * fresneleffect

  # compute refraction
  if depth < maxDepth and (obj.transparency > 0):
    var ior = 1.5
    let ce = Vec3.dot(ray.dir, normal) * -1.0
    
    ior = if inside: 1.0 / ior else: ior
    
    let
      eta = 1.0 / ior
      gf = (ray.dir + normal * ce) * eta
      sin_t1_2 = 1 - (ce * ce)
      sin_t2_2 = sin_t1_2 * (eta * eta)
    
    if sin_t2_2 < 1:
      let gc = normal * sqrt(1 - sin_t2_2)
      let refraction_direction = gf - gc
      var r: Ray
      r.org = point_of_hit - (normal * 1.0e-4)
      r.dir = refraction_direction
      color += trace(r, scene, depth + 1) * (1 - fresneleffect) * obj.transparency
  
  return color

type Pixels = ref array[0 .. (width * height * 3), byte]

proc render(pixels:var Pixels, scene:Scene) =
  let h: float = tan(((fov / 360) * (2 * Pi)) / 2) * 2
  let w: float = h * width / height
  
  for y in 0 .. height-1:
    for x in 0 .. width-1:
      let
        xx = x.float
        yy = y.float
        ww = width.float
        hh = height.float
      var
        dir {.noInit.}: Vec3
      
      dir.x = ((xx - (ww / 2.0)) / ww)  * w
      dir.y = (((hh / 2.0) - yy) / hh) * h
      dir.z = -1.0
      dir = Vec3.normalize(dir)
      
      var r: Ray
      r.dir = dir
      
      let pixel = trace(r, scene, 0)
      let i: int = (x * 3) + (y * width * 3)
      pixels[i] = min(pixel.x * 255, 255).byte
      pixels[i+1] = min(pixel.y * 255, 255).byte
      pixels[i+2] = min(pixel.z * 255, 255).byte

proc main =
  # create objects
  var scene = Scene.new()
  
  scene.objects = newSeq[Sphere]()
  scene.objects.add(Sphere.new(Vec3.new(0.0, -10002.0, -20.0), 10000, Vec3.new(0.8, 0.8, 0.8)))
  scene.objects.add(Sphere.new(Vec3.new(0.0, 2.0, -20.0), 4, Vec3.new(0.8, 0.5, 0.5), 0.5))
  scene.objects.add(Sphere.new(Vec3.new(5.0, 0.0, -15.0), 2, Vec3.new(0.3, 0.8, 0.8), 0.2))
  scene.objects.add(Sphere.new(Vec3.new(-5.0, 0.0, -15.0), 2, Vec3.new(0.3, 0.5, 0.8), 0.2))
  scene.objects.add(Sphere.new(Vec3.new(-2.0, -1.0, -10.0), 1, Vec3.new(0.1, 0.1, 0.1), 0.1, 0.8))
  
  scene.lights = newSeq[Light]()
  scene.lights.add(Light.new(Vec3.new(-10, 20, 30), Vec3.new(2, 2, 2)))
  
  # give the system a little time
  #GC.Collect()
  
  var data: Pixels
  data.new()
  
  echo "Starting test..."
  
  var start = (cpuTime() * 1000.0)
  data.render(scene)
  var duration = (cpuTime() * 1000.0) - start
  
  echo "Mil: ", duration.formatFloat(FFDecimal, 1)
  echo "Sec: ", (duration / 1000.0).formatFloat(FFDecimal, 2)
  
  #if WIN32
  #Win32EndOptimizedStopwatch()
  #endif
  
  # save image
  var stream = newFileStream("Image.raw", FMWrite)
  stream.writeData(cast[pointer](data), width * height * 3)

main()
