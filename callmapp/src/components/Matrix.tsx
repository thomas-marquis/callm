import React, { useEffect, useRef } from 'react';
import * as d3 from 'd3';
import Paper from '@mui/material/Paper';

interface MatrixVisualizationProps {
    label: string;
    matrix: number[][];
}

const Matrix: React.FC<MatrixVisualizationProps> = ({ matrix }) => {
    const ref = useRef<SVGSVGElement>(null);

    useEffect(() => {
        if (matrix.length === 0 || !ref.current) return;

        const displayNumbers = false;

        const svg = d3.select(ref.current);
        const containerWidth = ref.current.parentElement!.clientWidth;
        const containerHeight = ref.current.parentElement!.clientHeight;

        const rows = matrix.length;
        const cols = matrix[0].length;

        const cellSize = Math.min(containerWidth / cols, containerHeight / rows);

        const width = cols * cellSize;
        const height = rows * cellSize;

        svg.attr('width', '100%')
            .attr('height', '100%')
            .attr('viewBox', `0 0 ${width} ${height}`)
            .attr('preserveAspectRatio', 'xMidYMid meet');

        const minValue = d3.min(matrix, d => d3.min(d))!;
        const maxValue = d3.max(matrix, d => d3.max(d))!;

        const colorScale = d3.scaleLinear()
            .domain([minValue, maxValue])
            .range(['violet', 'indigo', 'blue', 'green', 'yellow', 'orange', 'red']);

        const groups = svg.selectAll('g')
            .data(matrix)
            .join('g')
            .attr('transform', (_, i) => `translate(0, ${i * cellSize})`);

        groups.selectAll('rect')
            .data(d => d)
            .join('rect')
            .attr('x', (_, i) => i * cellSize)
            .attr('width', cellSize)
            .attr('height', cellSize)
            .style('fill', d => colorScale(d))
            .style('stroke', 'black');

        if (displayNumbers) {
            groups.selectAll('text')
                .data(d => d)
                .join('text')
                .attr('x', (_, i) => i * cellSize + cellSize / 2)
                .attr('y', cellSize / 2)
                .attr('text-anchor', 'middle')
                .attr('dominant-baseline', 'middle')
                .text(d => d.toString());
        }

        const zoom = d3.zoom<SVGSVGElement, unknown>()
            .scaleExtent([1, Math.min(50, Math.max(containerWidth / width, containerHeight / height))])
            .on('zoom', (event) => {
                const { transform } = event;
                const strokeWidth = 1 / transform.k;
                svg.attr('transform', transform);
                svg.selectAll('rect')
                    .style('stroke-width', `${strokeWidth}px`);
            });

        svg.call(zoom);

        svg.on('contextmenu', (event) => {
            event.preventDefault(); // Empêcher le menu contextuel
        });

        const displacementCoefficient = Math.max(rows, cols);

        svg.call(d3.drag<SVGSVGElement, unknown>()
            .on('start', (event) => {
                console.log('start');
                if (event.button === 2) { // Bouton droit de la souris
                    svg.interrupt();
                }
            })
            .on('drag', (event) => {
                console.log('drag');
                if (event.button === 2) { // Bouton droit de la souris
                    const [x, y] = d3.pointer(event);
                    const scale = d3.zoomTransform(svg.node()!).k;
                    const speedFactor = displacementCoefficient / scale;
                    console.log(speedFactor);
                    zoom.translateBy(svg, x * speedFactor, y * speedFactor);
                }
            })
            .on("end", (event) => {
                console.log('end', event);
            }));

    }, [matrix]);

    return (
        <>
            <Paper
                elevation={3}
                style={{
                    position: 'absolute',
                    top: '64px',
                    left: 0,
                    right: 0,
                    bottom: 0,
                    display: 'flex',
                    justifyContent: 'center',
                    alignItems: 'center',
                    overflow: 'auto',
                }}
            >
                <svg ref={ref} style={{ width: '100%', height: '100%' }}></svg>
            </Paper>
        </>
    );
};

export default Matrix;
