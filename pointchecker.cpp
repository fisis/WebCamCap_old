#include "pointchecker.h"

#include "line.h"
#include "std_2d_vector.h"

using namespace glm;


size_t PointChecker::getNumOfPoints() const
{
    return numOfPoints;
}

void PointChecker::setNumOfPoints(const size_t &value)
{
    numOfPoints = value;
}

PointChecker::PointChecker()
{
    maxNoFrameDuration = 30;

    noFrameDuration = 0;
    wrongFrameDuration = 0;
    maxIndex = 0;
    numOfPoints = 1;
    state = PointCount::NO;
}

std::vector<Point> PointChecker::solvePointIDs(std::vector<vec3> points)
{
    std::vector<Point> pts;

    if(points.empty())
    {
        state = PointCount::NO;
        ++noFrameDuration;
        return pts;
    }

    switch (state) {
    case PointCount::NO:
    {
        pts = handleNo(points);
        break;
    }
    case PointCount::NOTENOUGH:
    {
        pts = handleNotEnough(points);
        break;
    }
    case PointCount::GOOD:
    {
        pts = handleGood(points);
        break;
    }
    case PointCount::TOOMANY:
    {
        pts = handleNotEnough(points);
        break;
    }
    default:
        break;
    }

    if(points.size() < lastPoints.size())
    {
        handleRemovedPoints(pts);
    }

    noFrameDuration = 0;
    lastPoints = pts;

    return pts;

/*
    std::cout << "distance map: " << std::endl;
    for (int i = 0; i < m.size(); i++)
    {
        for (int j = 0; j < m[i].size(); j++)
        {
            std::cout << m[i][j]  << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
*/

}

std::vector<Point> PointChecker::solvePointIDs(std::vector<vec2> points)
{
    std::vector<vec3> pts;

    for(size_t i = 0; i < points.size(); i++)
    {
        pts.push_back(glm::vec3(points[i], 0.0f));
    }

    return solvePointIDs(pts);
}

std::vector<Point> PointChecker::handleNo(std::vector<vec3> points)
{
    std::vector<Point> pts;

    if(points.size() < numOfPoints)
    {
        state = PointCount::NOTENOUGH;
    }
    else if(points.size() == numOfPoints)
    {
        state = PointCount::GOOD;
    }
    else if(points.size() > numOfPoints)
    {
        state = PointCount::TOOMANY;
    }

    if(noFrameDuration < maxNoFrameDuration && !lastGoodFrame.empty())
    {
        auto m = createDistanceMap(lastGoodFrame, points);

        my_solve(m);

        pts = addCoveredPoints(points, m);
    }
    else
    {
        for(size_t i = 0; i < points.size(); i++)
        {
            pts.push_back({nextUniqueIndex(i), points[i]});
        }
    }

    return pts;
}

std::vector<Point> PointChecker::handleNotEnough(std::vector<vec3> points)
{
    std::vector<Point> pts;

    std::vector< std::vector<double>> m = createDistanceMap(lastPoints, points);
    my_solve(m);

    for (int i = 0; i < m.size(); i++)
    {
        for (int j = 0; j < m[i].size(); j++)
        {
            std::cout << m[i][j]  << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    pts = addCoveredPoints(points,m);

    if(points.size() < numOfPoints)
    {
        state = PointCount::NOTENOUGH;

        addUncoveredPoints(points,m,pts);

    }
    else if(points.size() == numOfPoints)
    {
        state = PointCount::GOOD;

        addUncoveredPoints(points, m, pts);

    }
    else if(points.size() > numOfPoints)
    {
        state = PointCount::TOOMANY;

        addUncoveredPoints(points, m, pts);
    }

    return pts;
}

std::vector<Point> PointChecker::handleGood(std::vector<vec3> &points)
{
    std::vector<Point> pts;

    std::vector< std::vector<double>> m = createDistanceMap(lastPoints, points);
    my_solve(m);

    pts = addCoveredPoints(points,m);

    if(pts.size() == numOfPoints)
    {
        state = PointCount::GOOD;
        lastGoodFrame = pts;
    }
    else
    {
        state = PointCount::NOTENOUGH;
    }

    if(points.size() > numOfPoints)
    {
        for(size_t i = 0; i < points.size(); i++)
        {
            bool ok = false;
            for(size_t j = 0; j < pts.size(); j++)
            {
                if(points[i] == pts[j].position)
                {
                    ok = true;
                    break;
                }
            }
            if(!ok)
            {
                points.erase(points.begin()+i);
            }
        }
    }

    return pts;
}


std::vector< std::vector<double>> PointChecker::createDistanceMap(std::vector<Point> lastPoints, std::vector<vec3> points)
{
    std::vector< std::vector<double>> matrix;

    for(size_t i = 0; i < lastPoints.size(); i++)
    {
        std::vector<double> vec;
        for(size_t j = 0; j < points.size(); j++)
        {
            vec.push_back( glm::distance(lastPoints[i].position, points[j]));
        }
        matrix.push_back(vec);
    }

return matrix;
}

size_t PointChecker::nextUniqueIndex(int size)
{
    if(lastRemovedIDs.empty())
    {
        return size;
    }
    else
    {
        size_t val = lastRemovedIDs.front();
        lastRemovedIDs.pop_front();
        return val;
    }
}

void PointChecker::addUncoveredPoints(std::vector<vec3> points, std::vector<std::vector<double>> map, std::vector<Point> &pts)
{
    size_t rows = map.size();

    if(rows == 0)
    {
        return;
    }

    size_t cols = map[0].size();

    for(size_t i = 0; i < cols; i++)
    {
        bool ok  = false;
        for(size_t j = 0; j < rows; j++)
        {
            if(map[j][i] == 0)
            {
                ok = true;
                break;
            }
        }
        if(!ok)
        {
            pts.push_back({nextUniqueIndex(pts.size()), points[i]});
        }
    }
}

std::vector<Point> PointChecker::addCoveredPoints(std::vector<vec3> points, std::vector<std::vector<double>> map)
{
    std::vector<Point> pts;

    for(size_t i = 0; i < map.size(); i++)
    {
        for(size_t j = 0; j < map[i].size(); j++)
        {
            if(map[i][j] == 0)
            {
                pts.push_back({lastPoints[i].ID, points[j]});
            }
        }
    }

    return pts;
}

void PointChecker::handleRemovedPoints(std::vector<Point> points)
{
    for(size_t i = 0; i < lastPoints.size(); i++)
    {
        bool found = false;
        for(size_t j = 0; j < points.size(); j++)
        {
            if(lastPoints[i].ID == points[j].ID)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            lastRemovedIDs.push_back(lastPoints[i].ID);
        }
    }
}
