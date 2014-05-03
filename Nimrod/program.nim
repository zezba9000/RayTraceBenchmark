
import
  Math,
  Times,
  Streams,
  StrUtils

# ---

const width    = 1280
const height   = 720
const fov      = 45.0
const maxDepth = 6

# ---

type Vec3 =
  tuple
    x, y, z: float


{.push inline, noInit, noSideEffect.}

proc `-`(v:Vec3): Vec3 = (-v.x, -v.y, -v.z)

proc `+`(a, b:Vec3): Vec3 = (a.x + b.x, a.y + b.y, a.z + b.z)
proc `-`(a, b:Vec3): Vec3 = (a.x - b.x, a.y - b.y, a.z - b.z)

proc `*`(a, b:Vec3): Vec3 = (a.x * b.x, a.y * b.y, a.z * b.z)
proc `*`(a:Vec3, b:float): Vec3 = (a.x * b, a.y * b, a.z * b)

proc `/`(a, b:Vec3): Vec3 = (a.x / b.x, a.y / b.y, a.z / b.z)
proc `/`(a:Vec3, b:float): Vec3 = (a.x / b, a.y / b, a.z / b)

proc `+=`(this:var Vec3, v:Vec3) =
  this.x += v.x
  this.y += v.y
  this.z += v.z

{.pop.}

template new(T:typedesc[Vec3], x, y, z:float): Vec3 = (x, y, z)
template zero(T:typedesc[Vec3]): Vec3 = (0.0, 0.0, 0.0)

template dot(a, b:Vec3): float = (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
template normalize(v:Vec3): Vec3 = v / sqrt(dot(v, v))
  
# ---

type Ray {.byRef.} =
  object
    org, dir: Vec3


proc new(T:typedesc[Ray], org, dir:Vec3): Ray {.inline, noInit.} =
  result.org = org
  result.dir = dir

# ---

type Sphere =
  ref object
    center: Vec3
    radius: float
    color: Vec3
    reflection: float
    transparency: float


proc new(T:typedesc[Sphere], center:Vec3, radius:float, color:Vec3, refl:float = 0, trans:float = 0): Sphere =
  System.new(result)
  result.center = center
  result.radius = radius
  result.color = color
  result.reflection = refl
  result.transparency = trans


proc normal(this:Sphere, pos:Vec3): Vec3 {.inline, noInit.} =
  return normalize(pos - this.center)


proc intersect(this:Sphere, ray:Ray): bool {.noInit.} =
  let d = this.center - ray.org
  let a = dot(d, ray.dir)
  if a < 0: # opposite direction
    return false

  let b2 = dot(d, d) - (a * a)
  let r2 = this.radius * this.radius
  if b2 > r2: # perpendicular > r
    return false
  
  return true


proc intersect(this:Sphere, ray:Ray, distance:var float): bool {.noInit.} =
  distance = 0
  
  let d = this.center - ray.org
  let a = dot(d, ray.dir)
  if a < 0: # opposite direction
    return false
  
  let b2 = dot(d, d) - (a * a)
  let r2 = this.radius * this.radius
  if b2 > r2: # perpendicular > r
    return false
  
  let c = sqrt(r2 - b2)
  let near = a - c
  let far  = a + c
  # near < 0 means ray starts inside
  distance = if near < 0: far else: near
  
  return true

# ---

type Light =
  ref object
    position: Vec3
    color: Vec3


proc new(T:typedesc[Light], position, color:Vec3): Light =
  System.new(result)
  result.position = position
  result.color = color

# ---

type Scene =
  ref object
    objects: seq[Sphere]
    lights: seq[Light]


proc new(T:typedesc[Scene]): Scene = 
  System.new(result)
  result.objects = @[]
  result.lights = @[]

# ---

proc trace(this:Ray, scene:Scene, depth:int): Vec3 {.noInit.} = 
  var nearest = inf
  var obj: Sphere
  
  # search the scene for nearest intersection
  for o in scene.objects:
    var distance = inf
    if o.intersect(this, distance):
      if distance < nearest:
        nearest = distance
        obj = o
  
  if obj == nil:
    return Vec3.zero
  
  let hitPoint = this.org + (this.dir * nearest)
  var normal = obj.normal(hitPoint)
  var inside = false
  
  if dot(normal, this.dir) > 0:
    inside = true
    normal = -normal
  
  var color = Vec3.zero
  let reflRatio = obj.reflection
  
  for light in scene.lights:
    let lightDir = normalize(light.position - hitPoint)
    let tr = Ray.new(hitPoint + (normal * 1.0e-5), lightDir)
    
    # go through the scene check whether we're blocked from the lights
    var blocked = false
    for o in scene.objects:
      if o.intersect(tr):
        blocked = true
        break
    
    if not blocked:
      color += light.color * max(0.0, dot(normal, lightDir)) *
        obj.color * (1.0 - reflRatio)
  
  let rayNormDot = dot(this.dir, normal)
  let facing = max(0.0, -rayNormDot)
  let fresnel = reflRatio + ((1.0 - reflRatio) * pow((1.0 - facing), 5.0))
  
  # compute reflection
  if depth < maxDepth and reflRatio > 0:
    let reflDir = this.dir + (normal * 2 * rayNormDot * -1.0)
    let tr = Ray.new(hitPoint + (normal * 1.0e-5), reflDir)
    color += tr.trace(scene, depth + 1) * fresnel
  
  # compute refraction
  if depth < maxDepth and obj.transparency > 0:
    const iorDefault = 1.0 / 1.5
    let ior = if inside: iorDefault else: 1.5
    let ce = dot(this.dir, normal) * -1.0
    let eta = 1.0 / ior
    let gf = (this.dir + normal * ce) * eta
    let sin_t1_2 = 1.0 - (ce * ce)
    let sin_t2_2 = sin_t1_2 * (eta * eta)
    
    if sin_t2_2 < 1.0:
      let gc = normal * sqrt(1.0 - sin_t2_2)
      let refrDir = gf - gc
      let tr = Ray.new(hitPoint - (normal * 1.0e-4), refrDir)
      color += tr.trace(scene, depth + 1) * (1.0 - fresnel) * obj.transparency
  
  return color

# ---

const pixelsSize = width * height * 3
type Pixels {.byRef.} = array[0 .. pixelsSize, byte]

# ---

proc render(this:var Pixels, scene:Scene) =
  let h = tan(((fov / 360.0) * (2.0 * Pi)) / 2.0) * 2.0
  let w = h * width / height
  
  for y in 0 .. height-1:
    for x in 0 .. width-1:
      let wf: float = width
      let hf: float = height
      let dir = Vec3.new(
        ((x.float - (wf / 2.0)) / wf) * w,
        (((hf / 2.0) - y.float) / hf) * h,
        -1.0).normalize()
      
      let r = Ray.new(Vec3.zero, dir)
      let pixel = r.trace(scene, 0)
      let i: int = (x * 3) + (y * width * 3)
      
      this[i]   = min(pixel.x * 255, 255).byte
      this[i+1] = min(pixel.y * 255, 255).byte
      this[i+2] = min(pixel.z * 255, 255).byte


proc main =
  var scene = Scene.new()
  var image: Pixels
  
  scene.objects.add(Sphere.new(Vec3.new(0.0, -10002.0, -20.0), 10000, Vec3.new(0.8, 0.8, 0.8)))
  scene.objects.add(Sphere.new(Vec3.new(0.0, 2.0, -20.0), 4, Vec3.new(0.8, 0.5, 0.5), 0.5))
  scene.objects.add(Sphere.new(Vec3.new(5.0, 0.0, -15.0), 2, Vec3.new(0.3, 0.8, 0.8), 0.2))
  scene.objects.add(Sphere.new(Vec3.new(-5.0, 0.0, -15.0), 2, Vec3.new(0.3, 0.5, 0.8), 0.2))
  scene.objects.add(Sphere.new(Vec3.new(-2.0, -1.0, -10.0), 1, Vec3.new(0.1, 0.1, 0.1), 0.1, 0.8))
  
  scene.lights.add(Light.new(
    Vec3.new(-10.0, 20.0, 30.0),
    Vec3.new(2.0, 2.0, 2.0)))
  
  echo "Starting test..."
  
  let start = (cpuTime() * 1000.0)
  image.render(scene)
  let duration = (cpuTime() * 1000.0) - start
  
  echo "Mil: ", duration.formatFloat(FFDecimal, 1)
  echo "Sec: ", (duration / 1000.0).formatFloat(FFDecimal, 2)
  
  # save image
  var fs = newFileStream("Image.raw", FMWrite)
  fs.writeData(cast[pointer](addr image), pixelsSize)

# ---

when isMainModule:
  main()
