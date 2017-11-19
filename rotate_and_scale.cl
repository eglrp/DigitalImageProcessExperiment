#pragma OPENCL EXTENSION cl_nv_printf:enables
__kernel void rotate_and_scale(__global int *In, __global int* Out, int Width, int Height, double angle, double factor)
{
	double M_PI = 3.1415926;
	//  Degrees-to-radians constant 
	const double  DegreesToRadians = M_PI / 180.0;
	//�������
	double theta = angle * DegreesToRadians;// ��ת�ĽǶ�
	int x = get_global_id(0);
	int y = get_global_id(1);
	if (x <= Width && x >= 0 && y <= Height && y >= 0)
	{
		//������ת���v,w���п��ܴ���С��
		double real_v = x*cos(theta) + y*sin(theta) + cos(theta)*(-1)*(Width / 2) + sin(theta)*(-1)*(Height / 2);
		double real_w = (-1)*x*sin(theta) + y*cos(theta) + cos(theta)*(-1)*(Height / 2) - sin(theta)*(-1)*(Width / 2);
		//�����ź�ƽ��
		real_v = real_v / factor - (-1)*Width / 2;
		real_w = real_w / factor - (-1)*Height / 2;
		//��ÿһ��λ�ã�x,y��ʹ��(v,w) = T^(-1)(x,y)��������ͼ���е���Ӧλ�� 
		double value = 0;
		int v, w;
		//ȡ��������
		v = floor(real_v);
		w = floor(real_w);
		//if ((value = rs_normaliseXY(v, w, Width, Height)) != 0)//��û�����߽�Ļ�
		if (x >= max_x || x <= 0 || y >= max_y || y <= 0) //�����߽�Ĳ����ú�ɫ���
			value = 0;
		if (v >= 2 && v < Width - 2 && w >= 2 && w < Height - 2) {//��ֹԽ��														
				 //ʵ��˫�����ڲ�
				value = 0;
				//4*4��������
				for (int i = -1; i < 3; i++)
					for (int j = -1; j < 3; j++) {
						float V, W;
						float abs_x = abs(real_v - v - j);
						float a = -0.5;
						if (abs_x <= 1.0)
						{
							V = (a + 2)*pow(abs_x, 3) - (a + 3)*pow(abs_x, 2) + 1;
						}
						else if (abs_x < 2.0)
						{
							V = a*pow(abs_x, 3) - 5 * a*pow(abs_x, 2) + 8 * a*abs_x - 4 * a;
						}
						else
							V = 0.0;
						//
						abs_x = abs(real_w - w - i);
						if (abs_x <= 1.0)
						{
							W = (a + 2)*pow(abs_x, 3) - (a + 3)*pow(abs_x, 2) + 1;
						}
						else if (abs_x < 2.0)
						{
							W = a*pow(abs_x, 3) - 5 * a*pow(abs_x, 2) + 8 * a*abs_x - 4 * a;
						}
						else
							W = 0.0;

						value += (*(In + Width * (w + i) + (v + j)))*V*W;
					}
			}
		if (value > 255)
			value = 255;
		if (value < 0)
			value = 0;
		Out[y* Width + x] = value;
	}
}



