
import Math, Times, Streams, StrUtils

when defined(bigImgMT):
  import ThreadPool

# ---

const depthMax = 6
const fov = 45.0

when defined(bigImg) or defined(bigImgMT):
  const tiles  = 8
  const width  = 1280 * 8
  const height = 720 * 8
else:
  const width  = 1280
  const height = 720

# ---------- ---------- ---------- #

type Vec3 =
  tuple
    x, y, z: float

# ---

template zero(T:type Vec3): Vec3 = (0.0, 0.0, 0.0)

{.push inline, noInit.}

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

proc dot(a, b:Vec3): float = (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
proc normalize(v:Vec3): Vec3 = v / sqrt(dot(v, v))

converter toVec3[
  X: TReal|TInteger,
  Y: TReal|TInteger,
  Z: TReal|TInteger](
    T:tuple[x:X, y:Y, z:Z]): Vec3 = (float(T.x), float(T.y), float(T.z))

{.pop.}

# ---------- ---------- ---------- #

type Ray =
  object
    pos, dir: Vec3

type Sphere =
  ref object
    pos: Vec3
    color: Vec3
    radius: float
    refl, tran: float

type Light =
  ref object
    pos, color: Vec3

type Scene =
  ref object
    objects: seq[Sphere]
    lights: seq[Light]

# ---

proc new(T:type Scene): Scene =
  new result
  result.objects = @[]
  result.lights = @[]

# ---

{.push noInit.}

proc normal(this:Sphere, pos:Vec3): Vec3 {.inline.} =
  return normalize(pos - this.pos)


proc intersect(this:Sphere, ray:Ray): bool =
  let d = this.pos - ray.pos
  let a = dot(d, ray.dir)
  if a < 0: # opposite direction
    return false

  let b2 = dot(d, d) - (a * a)
  let r2 = this.radius * this.radius
  if b2 > r2: # perpendicular > r
    return false
  
  return true


proc intersect(this:Sphere, ray:Ray, distance:var float): bool =
  distance = 0
  
  let d = this.pos - ray.pos
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


proc trace(this:Ray, scene:Scene, depth:int): Vec3 = 
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
  
  let hitPoint = this.pos + (this.dir * nearest)
  var normal = obj.normal(hitPoint)
  var inside = false
  
  if dot(normal, this.dir) > 0:
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
  
  let rayNormDot = dot(this.dir, normal)
  let facing = max(0, -rayNormDot)
  let fresnel = reflRatio + ((1 - reflRatio) * pow((1 - facing), 5))
  
  # compute reflection
  if depth < depthMax and reflRatio > 0:
    let reflDir = this.dir + (normal * 2 * rayNormDot * -1)
    let tr = Ray(pos:hitPoint + (normal * 1.0e-5), dir:reflDir)
    color += tr.trace(scene, depth + 1) * fresnel
  
  # compute refraction
  if depth < depthMax and obj.tran > 0:
    const iorDefault = 1 / 1.5
    let ior = if inside: iorDefault else: 1.5
    let ce = dot(this.dir, normal) * -1
    let eta = 1 / ior
    let gf = (this.dir + normal * ce) * eta
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

const pixmapSize = width * height * 3
type Pixmap = array[pixmapSize, byte]

# ---

proc renderRegion(this:ptr Pixmap, scene:Scene, w, h:float, sx, sy, ex, ey:int) =
  let wf: float = width
  let hf: float = height
  
  for y in sy .. ey:
    for x in sx .. ex:
      let dir: Vec3 = (
        x: ((float(x) - (wf / 2)) / wf) * w,
        y: (((hf / 2) - float(y)) / hf) * h,
        z: -1
      ).normalize()
      
      let ray = Ray(pos:Vec3.zero, dir:dir)
      let pixel = ray.trace(scene, 0)
      let index: int = (x * 3) + (y * width * 3)
      
      this[index]   = byte min(pixel.x * 255, 255)
      this[index+1] = byte min(pixel.y * 255, 255)
      this[index+2] = byte min(pixel.z * 255, 255)


proc render(this:ref Pixmap, scene:Scene) =
  let h = tan(((fov / 360) * (2 * Pi)) / 2) * 2
  let w = h * width / height
  let pthis = cast[ptr Pixmap](this)
  
  when not defined(bigImgMT):
    renderRegion(pthis, scene, w, h, 0, 0, width-1, height-1)
  else:
    let sizeW = int(width / tiles)
    let sizeH = int(height / tiles)
    parallel:
      for y in 0 .. <tiles:
        for x in 0 .. <tiles:
          let sx = x * sizeW
          let sy = y * sizeH
          let ex = (sx + sizeW) - 1
          let ey = (sy + sizeH) - 1
          spawn renderRegion(pthis, scene, w, h, sx, sy, ex, ey)

# ---------- ---------- ---------- #

proc main =
  ## raytraces a simple scene and saves an image
  let scene = new Scene
  let image = new Pixmap
  
  # add geometry
  scene.objects.add Sphere(pos:(0, -10002, -20), radius:10000, color:(0.8, 0.8, 0.8))
  scene.objects.add Sphere(pos:( 0,  2, -20), radius:4, color:(0.8, 0.5, 0.5), refl:0.5)
  scene.objects.add Sphere(pos:( 5,  0, -15), radius:2, color:(0.3, 0.8, 0.8), refl:0.2)
  scene.objects.add Sphere(pos:(-5,  0, -15), radius:2, color:(0.3, 0.5, 0.8), refl:0.2)
  scene.objects.add Sphere(pos:(-2, -1, -10), radius:1, color:(0.1, 0.1, 0.1), refl:0.1, tran:0.8)
  
  # add light
  scene.lights.add Light(pos:(-10, 20, 30), color:(2, 2, 2))
  
  # run test
  echo "Starting test..."
  
  let begin = epochTime()
  
  when defined(averageRuns):
    for i in 1..20:
      image.render(scene)
  else:
    image.render(scene)
    
  let finish = epochTime()
  
  var elapsedTime = finish - begin
  
  when defined(averageRuns):
    elapsedTime /= 20
  
  echo "Seconds: ", (elapsedTime).formatFloat(FFDecimal, 3)
  
  when not defined(noSave):
    # save image
    let fs = newFileStream("image.rgb", FMWrite)
    fs.writeData(cast[pointer](image), pixmapSize)
    fs.close()

# ---

when isMainModule:
  main() # run program
